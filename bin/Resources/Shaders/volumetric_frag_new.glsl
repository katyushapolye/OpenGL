#version 440 core
out vec4 FragColor;
  
in vec2 TexCoords;
in mat4 inverseViewMat;
in mat4 inverseProjectionMat;
in mat4 inverseModelMat;

//screen buffers
uniform sampler2D screenTexture;
uniform sampler2D screenDepth;
uniform samplerCube skybox;
uniform float time;

//ray march parameters
#define dt  0.005
#define EPS 0.001
#define MAX_RANGE  30

//volume parameters
uniform vec3 volumeCenter;
uniform vec3 volumeDimension;
uniform sampler3D volumeDensity;
uniform vec3 scatteringCoefficient;
uniform float densityMultiplier;

//Lights
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float radius;
};

struct DirectionalLight {
    vec3 direction;
    float intensity;
    vec3 color;
    mat4 lightMat;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float intensity;
    vec3 color;
    float theta;
};

#define NR_POINT_LIGHTS 16
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform int pointLightCount;

#define NR_DIRECTIONAL_LIGHTS 4
uniform DirectionalLight dirLights[NR_DIRECTIONAL_LIGHTS];
uniform int directionalLightCount;

#define NR_SPOT_LIGHTS 16
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
uniform int spotLightCount;

struct Ray {
    vec3 pos;
    vec3 dir;
};

struct Hit{
    vec3 pos;
    vec3 normal;
    float dist;
    float totalDensity;
    float reflectionStrength;
    float refractionStrength;
    bool found;
};

layout(std140, binding = 1) uniform Camera  {
    vec3 cameraPos;
    vec3 cameraRot;
    vec2 nearFar;
};

layout(std140, binding = 0) uniform Matrixes  {
    mat4 viewMat;
    mat4 projectionMat;
};

uniform mat4 modelMat; // Transform matrix for the volumetric




vec3 worldToLocal(vec3 worldPos) {
    return (inverseModelMat * vec4(worldPos, 1.0)).xyz;
}

vec3 localToWorld(vec3 localPos) {
    return (modelMat * vec4(localPos, 1.0)).xyz;
}


vec3 worldDirToLocal(vec3 worldDir) {
    return normalize((inverseModelMat * vec4(worldDir, 0.0)).xyz);
}


vec3 localDirToWorld(vec3 localDir) {
    return normalize((modelMat * vec4(localDir, 0.0)).xyz);
}

float CubeSDF(vec3 worldPos) {
    // Transform to local space for SDF evaluation
    vec3 localPos = worldToLocal(worldPos);
    vec3 halfExtents = volumeDimension * 0.5;
    vec3 relativePos = localPos - volumeCenter;
    vec3 q = abs(relativePos) - halfExtents;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

vec3 worldToCubeMap(vec3 worldPos) {
    // Transform to local space first
    vec3 localPos = worldToLocal(worldPos);
    vec3 cubeMin = volumeCenter - 0.5 * volumeDimension;
    vec3 cubeMax = volumeCenter + 0.5 * volumeDimension;
    vec3 uvw = (localPos - cubeMin) / (cubeMax - cubeMin);
    return clamp(uvw, 0.001, 0.999);
}


bool RayBoxIntersect(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax, out float tNear, out float tFar)
{
    // Transform ray to local space
    vec3 localOrigin = worldToLocal(rayOrigin);
    vec3 localDir = worldDirToLocal(rayDir);
    
    vec3 invDir = 1.0 / localDir;
    vec3 t0s = (boxMin - localOrigin) * invDir;
    vec3 t1s = (boxMax - localOrigin) * invDir;

    vec3 tsmaller = min(t0s, t1s);
    vec3 tbigger  = max(t0s, t1s);

    tNear = max(max(tsmaller.x, tsmaller.y), tsmaller.z);
    tFar  = min(min(tbigger.x, tbigger.y), tbigger.z);

    return tFar >= max(tNear, 0.0);
}

bool isInsideFluid(vec3 worldPos) {
    vec3 uvw = worldToCubeMap(worldPos);
    bool insideBounds = all(greaterThanEqual(uvw, vec3(0.0))) && all(lessThanEqual(uvw, vec3(1.0)));

    if (!insideBounds)
        return false;

    float d = texture(volumeDensity, uvw).r;
    return d > 0.01*densityMultiplier;
}



vec3 worldToScreen(vec3 world, mat4 view, mat4 proj) {
    vec4 clip = proj * view * vec4(world, 1.0);
    vec3 ndc = clip.xyz / clip.w;
    return ndc * 0.5 + 0.5; 
}

vec3 screenToWorld(vec3 screen) {
    vec3 ndc = screen * 2.0 - 1.0;
    vec4 clip = vec4(ndc, 1.0);
    vec4 viewPos = inverseProjectionMat * clip;
    viewPos /= viewPos.w;
    vec4 worldPos = inverseViewMat * viewPos;
    return worldPos.xyz;
}





vec3 ScreenSpaceSample(vec3 pos, vec3 dir)
{
      vec3 screenPos = worldToScreen(pos, viewMat, projectionMat);
    float depthAtPos = texture(screenDepth, screenPos.xy).r;
    const float initialMargin = 0.05;
    if (screenPos.z > depthAtPos + initialMargin) {
        if (screenPos.x >= 0.0 && screenPos.x <= 1.0 && screenPos.y >= 0.0 && screenPos.y <= 1.0) {
            return texture(screenTexture, screenPos.xy).rgb;
        } else {
            return texture(skybox, dir).rgb ;
        }
    }
    //maybe using binary search for precision
    vec3 rayPos = pos;
    float traveled = 0.0;
    const float marchStep = dt;
    const float maxSteps = MAX_RANGE / dt;
    const float depthMargin = 0.0001;
    for (int i = 0; i < int(maxSteps); ++i)
    {
        rayPos += dir * marchStep;
        traveled += marchStep;
        vec3 scr = worldToScreen(rayPos, viewMat, projectionMat);
        if (scr.x < 0.0 || scr.x > 1.0 || scr.y < 0.0 || scr.y > 1.0) {
            if (traveled > MAX_RANGE) break;
            continue;
        }
        float sceneDepth = texture(screenDepth, scr.xy).r;
        if (scr.z >= sceneDepth - depthMargin) {
            vec3 color = texture(screenTexture, scr.xy).rgb;
            return color;
        }
        if (traveled > MAX_RANGE) break;
    }
    return texture(skybox, dir).rgb;
}

float getDensity(vec3 worldPos) {
    vec3 uvw = worldToCubeMap(worldPos);
    if(!all(greaterThanEqual(uvw, vec3(0.0))) || !all(lessThanEqual(uvw, vec3(1.0))))
        return 0.0;
    return texture(volumeDensity, uvw).r;
}

// Helper function to compute gradient at a specific UVW coordinate
vec3 getGradientAt(vec3 uvw, float texelUv) {
    // central differences: (f(x+dx)-f(x-dx)) / (2*dx)
    float sxp = texture(volumeDensity, uvw + vec3(texelUv, 0.0, 0.0)).r;
    float sxn = texture(volumeDensity, uvw - vec3(texelUv, 0.0, 0.0)).r;
    float dxx = (sxp - sxn) / (2.0 * texelUv);

    float syp = texture(volumeDensity, uvw + vec3(0.0, texelUv, 0.0)).r;
    float syn = texture(volumeDensity, uvw - vec3(0.0, texelUv, 0.0)).r;
    float dyy = (syp - syn) / (2.0 * texelUv);

    float szp = texture(volumeDensity, uvw + vec3(0.0, 0.0, texelUv)).r;
    float szn = texture(volumeDensity, uvw - vec3(0.0, 0.0, texelUv)).r;
    float dzz = (szp - szn) / (2.0 * texelUv);

    return vec3(dxx, dyy, dzz);
}

vec3 getDensityGradient(vec3 worldPos) {
    vec3 uvw = worldToCubeMap(worldPos);

    // texel size in UV space
    float texelUv = 1.0 / float(128);

    // optionally clamp to avoid sampling outside the volume
    uvw = clamp(uvw, vec3(texelUv), vec3(1.0 - texelUv));

    // base central-difference gradient
    vec3 localGradient = getGradientAt(uvw, texelUv);

    // OPTIONAL: apply a simple 3-sample smoothing to reduce noise (uncomment if desired)
    // vec3 g2 = getGradientAt(uvw + vec3(texelUv,0,0), texelUv);
    // vec3 g3 = getGradientAt(uvw - vec3(texelUv,0,0), texelUv);
    // localGradient = (localGradient + g2 + g3) / 3.0;

    // Build proper normal matrix (transpose(inverse(modelMat))) for transforming normals
    // If you already have inverseModelMat, normalMat = mat3(transpose(inverseModelMat));
    mat3 normalMat = mat3(transpose(inverseModelMat)); // or: mat3(transpose(inverse(modelMat)))

    // Transform gradient (gradient acts like a normal) from texture/model space to world space
    vec3 worldGradient = normalMat * localGradient;

    // Avoid normalizing a near-zero vector (returns zero vector for near-empty regions)
    float len = length(worldGradient);
    if (len < 1e-6) return vec3(0.0); // no normal where gradient is zero

    return normalize(worldGradient);
}

vec3 marchNew(Ray r, vec3 sceneColor, float maxRayDistance)
{
    vec3 cubeMin = volumeCenter - 0.5 * volumeDimension;
    vec3 cubeMax = volumeCenter + 0.5 * volumeDimension;
    float tNear, tFar;
    bool hit = RayBoxIntersect(r.pos, r.dir, cubeMin, cubeMax, tNear, tFar);
    
    if (!hit || tFar < 0.0)
        return sceneColor;
    
    // Calculate scene depth relative to ray origin
    float distanceToRayOrigin = length(r.pos - cameraPos);
    float relativeMaxDistance = maxRayDistance - distanceToRayOrigin;
    
    tNear = max(tNear, 0.0);
    
    if (tNear > relativeMaxDistance)
        return sceneColor;
    
    // Clamp tFar to scene depth BEFORE calculating maxDist
    tFar = min(tFar, relativeMaxDistance);
    float maxDist = tFar - tNear;
    
    r.pos += r.dir * tNear;
    
    int steps = 0;
    float dist = 0.0;
    float accumulated = 0.0;
    bool onFluid = false;
    bool onAir = true;

    float n1 = 1.0;
    float n2 = 1.33;
    int refractionCount = 0;
    const int MAX_REFRACTIONS = 4;

    vec3 lastPos = r.pos;
    float lastDensity = 0;
    
    while(dist < maxDist && steps < 10000) {
        //Check depth BEFORE stepping to catch geometry immediately
        vec3 testPos = r.pos + r.dir * dt;
        float currentDist = length(testPos - cameraPos);
        if (currentDist > maxRayDistance) {
            break;
        }
        
        r.pos = testPos;
        dist += dt;
        steps++;
        
        if(CubeSDF(r.pos) > 0.0) {
            break;
        }
        
    float iso = 0.01 * densityMultiplier;
    float prevD = lastDensity;
    float currD = getDensity(r.pos);   

    bool crossed = (prevD > iso && currD <= iso) ||
                   (prevD < iso && currD >= iso);

    if(isInsideFluid(r.pos)){
        accumulated += currD * dt * densityMultiplier;
    }

    if(crossed) {
        // interpolate exact crossing point
        float t = (iso - prevD) / (currD - prevD);
        t = clamp(t, 0.0, 1.0);
        vec3 hitPos = mix(lastPos, r.pos, t);

        int factor = 1;
        if(onAir) {
            onAir = false;
            onFluid = true;
            n1 = 1.0;
            n2 = 1.33;
            factor = -1;

        } else if(onFluid) {
            onAir = true;
            onFluid = false;
            n1 = 1.33;
            n2 = 1.0;
            factor = 1;
        }   

        // compute normal AT the surface
        // compute refraction
        vec3 normal = normalize(getDensityGradient(hitPos));
        if (dot(normal, r.dir) > 0.0)
            normal = -normal;

        vec3 refractDir =  refract(r.dir, normal, n1/n2);

        if(length(refractDir) < 0.1 || refractionCount >= MAX_REFRACTIONS) {
            // total internal reflection or too many refractions, reflect instead
            vec3 reflectDir = reflect(r.dir, normal);
            r.pos = hitPos ;
            r.dir = reflectDir;
            break;
        } else {
            // refraction occurred
            r.pos = hitPos ;
            r.dir = refractDir;
            refractionCount++;

        }
        //return (normal + 1.0) * 0.5;   // debug output
    }


    lastPos = r.pos;
    lastDensity = currD;
    }
    // move forward in the algorithm


    
    
    return exp(-accumulated*densityMultiplier*scatteringCoefficient)*ScreenSpaceSample(r.pos, r.dir);
}

float linearizeDepth(float depthNDC, float near, float far)
{
    float z = depthNDC * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {

    vec4 ndc = vec4(TexCoords * 2.0 - 1.0, -1.0, 1.0);
    vec4 viewPos = inverseProjectionMat * ndc;        
    viewPos /= viewPos.w;
    vec3 rayDir = normalize((inverseViewMat * vec4(viewPos.xyz, 0.0)).xyz);
    
    Ray r;
    r.pos = cameraPos;
    r.dir = rayDir;

    float depthSample = texture(screenDepth, TexCoords).r;
    float linearDepth = linearizeDepth(depthSample, nearFar.x, nearFar.y);
    
    vec3 viewDir = normalize(viewPos.xyz);
    float maxRayDistance = linearDepth / abs(viewDir.z);
    vec3 sceneColor = texture(screenTexture, TexCoords).rgb;
   
    vec3 finalColor = marchNew(r, sceneColor, linearDepth);
   
    FragColor = vec4(finalColor, 1.0);
}