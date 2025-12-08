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
#define dt  0.01
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

/*
Hit findNextSurface(vec3 origin, vec3 dir, bool findNextFluidEntry, float maxDist)
{
    Hit info;
    info.found = false;
    info.normal = vec3(0.0);
    info.totalDensity = 0.0;
    info.dist = 0.0;
    info.pos = origin;

    if (dot(dir, dir) < 0.01) return info;

    float tNear, tFar;
    bool hitBox = RayBoxIntersect(origin, dir,
        volumeCenter - 0.5 * volumeDimension,
        volumeCenter + 0.5 * volumeDimension,
        tNear, tFar);

    if (!hitBox) return info;

    float stepSize = dt;
    tNear = max(tNear, 0.0);
    origin += dir * (tNear + 0.01);

    bool hasExitedFluid = !isInsideFluid(origin);
    bool hasEnteredFluid = isInsideFluid(origin);
    vec3 lastPosInFluid = origin;

    float tEnd = min(tFar - tNear, maxDist);

    for (float t = 0.0; t < tEnd; t += stepSize)
    {
        vec3 pos = origin + dir * t;
        vec3 uvw = worldToCubeMap(pos);
        
        if (!all(greaterThanEqual(uvw, vec3(0.0))) || !all(lessThanEqual(uvw, vec3(1.0)))) {
            continue;
        }
        
        float d = texture(volumeDensity, uvw).r;
        bool insideFluid = (d > 0.01);

        info.totalDensity += d * stepSize * densityMultiplier;

        if (insideFluid) {
            hasEnteredFluid = true;
            lastPosInFluid = pos;
        }
        if (!insideFluid)
            hasExitedFluid = true;

        bool found;
        if (findNextFluidEntry)
            found = insideFluid && hasExitedFluid;
        else
            found = hasEnteredFluid && !insideFluid;

        if (found) {
            info.found = true;
            info.pos = lastPosInFluid;
            info.dist = t;
            
            vec3 grad = getDensityGradient(info.pos);
            if(length(grad) > 0.001) {
                info.normal = -normalize(grad);
            } else {
                info.normal = -normalize(dir);
            }
            return info;
        }
    }

    return info;
}
*/

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

/*
float densityAlongDirection(vec3 pos, vec3 dir) {
    vec3 cubeMin = volumeCenter - 0.5 * volumeDimension;
    vec3 cubeMax = volumeCenter + 0.5 * volumeDimension;
    
    float tNear, tFar;
    bool hit = RayBoxIntersect(pos, dir, cubeMin, cubeMax, tNear, tFar);
    
    if (!hit || tFar < 0.0) {
        return 0.0;
    }
    
    tNear = max(tNear, 0.0);
    float marchDist = tNear;
    float maxDist = min(tFar, MAX_RANGE);
    
    vec3 marchPos = pos + dir * tNear;
    float density = 0.0;
    
    while(marchDist < maxDist){
        marchPos += dir * dt;
        marchDist += dt;
        
        vec3 uvw = worldToCubeMap(marchPos);
        if (all(greaterThanEqual(uvw, vec3(0.0))) && all(lessThanEqual(uvw, vec3(1.0)))) {
            density += texture(volumeDensity, uvw).r * dt;
        }
    }
    return density;
}

vec3 getLightInFluid(vec3 pos, vec3 dir)
{
    vec3 screenPos = worldToScreen(pos, viewMat, projectionMat);
    float depthAtPos = texture(screenDepth, screenPos.xy).r;
    const float initialMargin = 0.005;

    if (screenPos.z > depthAtPos + initialMargin) {
        if (screenPos.x >= 0.0 && screenPos.x <= 1.0 && screenPos.y >= 0.0 && screenPos.y <= 1.0) {
            return texture(screenTexture, screenPos.xy).rgb * dirLights[0].color;
        } else {
            return texture(skybox, dir).rgb * dirLights[0].color;
        }
    }

    vec3 rayPos = pos;
    float traveled = 0.0;
    const float marchStep = dt;
    const float maxSteps = MAX_RANGE / dt;
    const float depthMargin = 0.01;

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
            return color * dirLights[0].color;
        }

        if (traveled > MAX_RANGE) break;
    }

    return texture(skybox, dir).rgb * dirLights[0].color;
}

float getReflectance(vec3 incidence, vec3 normal, float n1, float n2) {
    float refractionIndex = n1 / n2;
    float cosIn = -dot(incidence, normal);
    float sinSqrt = refractionIndex * refractionIndex * (1.0 - cosIn * cosIn);
    if(sinSqrt >= 1.0) { return 1.0; }
    float cosRefract = sqrt(1.0 - sinSqrt);
    float sqrtRayPerp = (n1 * cosIn - n2 * cosRefract) / (n1 * cosIn + n2 * cosRefract);
    float sqrtRayParal = (n2 * cosIn - n1 * cosRefract) / (n2 * cosIn + n1 * cosRefract);
    return (sqrtRayPerp * sqrtRayPerp + sqrtRayParal * sqrtRayParal) / 2.0;
}

vec3 march(Ray r, vec3 sceneColor)
{
    vec3 cubeMin = volumeCenter - 0.5 * volumeDimension;
    vec3 cubeMax = volumeCenter + 0.5 * volumeDimension;
    float tNear, tFar;
    bool hit = RayBoxIntersect(r.pos, r.dir, cubeMin, cubeMax, tNear, tFar);
   
    if (!hit || tFar < 0.0)
        return sceneColor;
   
    tNear = max(tNear, 0.0);
    r.pos += r.dir * (tNear + 0.01);
   
    vec3 transmittance = vec3(1.0);
    vec3 accumulatedLight = vec3(0.0);
    bool travellingThroughFluid = isInsideFluid(r.pos);
   
    const int MAX_REFRACTIONS = 1;
    
    for(int i = 0; i < MAX_REFRACTIONS; i++){
        bool searchForFluidEntry = !travellingThroughFluid;
        Hit surfaceInfo = findNextSurface(r.pos, r.dir, searchForFluidEntry, 50.0);
        if(!surfaceInfo.found || length(surfaceInfo.normal) < 0.1 && i == 0){
            return sceneColor;
        }
        
        if(!surfaceInfo.found || length(surfaceInfo.normal) < 0.1){
            break;
        }
        
        vec3 segmentTransmittance = exp(-surfaceInfo.totalDensity * scatteringCoefficient);
        transmittance *= segmentTransmittance;
        
        r.pos = surfaceInfo.pos;
        vec3 normal = surfaceInfo.normal;
        
        if(dot(normal, r.dir) > 0.0) {
            normal = -normal;
        }
        
        float iorA = travellingThroughFluid ? 1.33 : 1.0;
        float iorB = travellingThroughFluid ? 1.0 : 1.33;
        float eta = iorA / iorB;
        
        vec3 refractDir = refract(normalize(r.dir), normal, eta);
        vec3 reflectDir = reflect(normalize(r.dir), normal);
        
        float reflectance = getReflectance(r.dir, normal, iorA, iorB);
        float refractWeight = 1.0 - reflectance;
        
        if(reflectance >= 0.99 || length(refractDir) < 0.1) {
            refractDir = reflectDir;
            refractWeight = 0.0;
            reflectance = 1.0;
        }
        
        float densityAlongRefract = densityAlongDirection(r.pos, refractDir);
        float densityAlongReflect = densityAlongDirection(r.pos, reflectDir);
        
        float refractImportance = densityAlongRefract * refractWeight;
        float reflectImportance = densityAlongReflect * reflectance;
        
        bool traceRefractedRay = refractImportance > reflectImportance;
        
        if(traceRefractedRay) {
            travellingThroughFluid = !travellingThroughFluid;
        }
        
        vec3 lessImportantDir = traceRefractedRay ? reflectDir : refractDir;
        float lessImportantWeight = traceRefractedRay ? reflectance : refractWeight;
        float lessImportantDensity = traceRefractedRay ? densityAlongReflect : densityAlongRefract;
        vec3 lessImportantTrans = exp(-lessImportantDensity * scatteringCoefficient);
        
        vec3 envLight = getLightInFluid(r.pos, lessImportantDir);
        accumulatedLight += envLight * transmittance * lessImportantTrans * lessImportantWeight;
        
        r.dir = traceRefractedRay ? refractDir : reflectDir;
        r.pos += r.dir * 0.02;
        transmittance *= (traceRefractedRay ? refractWeight : reflectance);
        
        if(max(transmittance.r, max(transmittance.g, transmittance.b)) < 0.01) {
            break;
        }
    }
    
    float remainingDensity = densityAlongDirection(r.pos, r.dir);
    vec3 remainingTransmittance = exp(-remainingDensity * scatteringCoefficient);
    vec3 finalEnvLight = getLightInFluid(r.pos, r.dir);
    accumulatedLight += finalEnvLight * transmittance * remainingTransmittance;
    
    return accumulatedLight;
}
*/

vec3 getLightInFluid(vec3 pos, vec3 dir)
{
    vec3 screenPos = worldToScreen(pos, viewMat, projectionMat);
    float depthAtPos = texture(screenDepth, screenPos.xy).r;
    const float initialMargin = 0.00005;

    if (screenPos.z > depthAtPos + initialMargin) {
        if (screenPos.x >= 0.0 && screenPos.x <= 1.0 && screenPos.y >= 0.0 && screenPos.y <= 1.0) {
            return texture(screenTexture, screenPos.xy).rgb * dirLights[0].color;
        } else {
            return texture(skybox, dir).rgb * dirLights[0].color;
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
            return color * dirLights[0].color;
        }

        if (traveled > MAX_RANGE) break;
    }

    return texture(skybox, dir).rgb * dirLights[0].color;
}

float getDensity(vec3 worldPos) {
    vec3 uvw = worldToCubeMap(worldPos);
    if(!all(greaterThanEqual(uvw, vec3(0.0))) || !all(lessThanEqual(uvw, vec3(1.0))))
        return 0.0;
    return texture(volumeDensity, uvw).r*densityMultiplier;
}

// Helper function to compute gradient at a specific UVW coordinate
vec3 getGradientAt(vec3 uvw, float texelSize) {
    float dx = texture(volumeDensity, uvw + vec3(texelSize, 0, 0)).r*densityMultiplier; 
             - texture(volumeDensity, uvw - vec3(texelSize, 0, 0)).r*densityMultiplier;;
    float dy = texture(volumeDensity, uvw + vec3(0, texelSize, 0)).r*densityMultiplier; 
             - texture(volumeDensity, uvw - vec3(0, texelSize, 0)).r*densityMultiplier;;
    float dz = texture(volumeDensity, uvw + vec3(0, 0, texelSize)).r*densityMultiplier; 
             - texture(volumeDensity, uvw - vec3(0, 0, texelSize)).r*densityMultiplier;;
    return vec3(dx, dy, dz);
}


vec3 getDensityGradient(vec3 worldPos) {
    vec3 uvw = worldToCubeMap(worldPos);
    float texelSize = 1.0 / 512.0;
    float offset = texelSize;
    
    vec3 blurredGradient = vec3(0.0);
    
    // 3x3x3 Gaussian weights (normalized)
    for(int z = -1; z <= 1; z++) {
        for(int y = -1; y <= 1; y++) {
            for(int x = -1; x <= 1; x++) {
                vec3 sampleOffset = vec3(x, y, z) * offset;
                vec3 grad = getGradientAt(uvw + sampleOffset, texelSize);
                
                // Gaussian weight based on distance
                float weight = exp(-float(x*x + y*y + z*z) * 0.5);
                blurredGradient += grad * weight;
            }
        }
    }
    
    vec3 localGradient = blurredGradient / (2.0 * texelSize * length(volumeDimension));
    
    return normalize(mat3(transpose(inverseModelMat)) * localGradient);
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
    bool onInterface = false;  
    float n1 = 1.0;
    float n2 = 1.33;
    
    while(dist < maxDist && steps < 10000) {
        // Check depth BEFORE stepping to catch geometry immediately
        vec3 testPos = r.pos + r.dir * dt;
        float currentDist = length(testPos - cameraPos);
        
        // If we're about to go beyond scene geometry, stop
        if (currentDist > maxRayDistance) {
            break;
        }
        
        r.pos = testPos;
        dist += dt;
        steps++;
        
        if(CubeSDF(r.pos) > 0.0) {
            break;
        }
        
        if(onFluid == true){
            accumulated += dt*getDensity(r.pos)*densityMultiplier;
            if(isInsideFluid(r.pos) == false){
                onFluid = false;
                n1 = 1.0;
                n2 = 1.33;
                onAir = true;
                onInterface = true; 
            }
        }
        
        if(onAir == true){
            if(isInsideFluid(r.pos) == true){
                n1 = 1.33;
                n2 = 1.0;
                onFluid = true;
                onAir = false;
                onInterface = true; 
            }
        }
        
        if(onInterface){
            onInterface = false;
            vec3 normal = normalize(-getDensityGradient(r.pos));
            vec3 refractDir = refract(r.dir, normal, n1/n2);
            
            if(length(refractDir) < 0.5) {
                r.dir = reflect(r.dir, normal);
                //return vec3(0.0,1.0,0.0); //reflection
                break;
            }
            else{
                r.dir = refractDir;
                //return vec3(0.0,0.0,1.0);
                // After refraction, we can't rely on maxDist anymore
                // The continuous depth check above will handle it
            }
        }
    }
    
    return exp(-accumulated*densityMultiplier*scatteringCoefficient)*getLightInFluid(r.pos, r.dir);
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