#version 440 core
out vec4 FragColor;
  
in vec2 TexCoords;
in mat4 inverseViewMat;
in mat4 inverseProjectionMat;

//screen buffers
uniform sampler2D screenTexture;
uniform sampler2D screenDepth;
uniform samplerCube skybox; //TEX UNIT 0
uniform float time;

//ray march parameters
#define dt  0.03
#define EPS 0.001
#define MAX_RANGE  50
//volume parameters
uniform vec3 volumeCenter;
uniform vec3 volumeDimension;
uniform sampler3D volumeDensity;

uniform vec3 scatteringCoefficient;

#define densityMultiplier 0.1;

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


float CubeSDF(vec3 pos) {
    vec3 halfExtents = volumeDimension * 0.5;
    vec3 localPos = pos - volumeCenter;
    vec3 q = abs(localPos) - halfExtents;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
};



layout(std140, binding = 0) uniform Matrixes  {
    mat4 viewMat;
    mat4 projectionMat;

};





/*

float hash(float n) {
    return fract(sin(n)*43758.5453);
}
// Base 3D noise function
float noise(in vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
    
    float n = p.x + p.y*57.0 + 113.0*p.z;
    
    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return res;
}
vec3 marchRay_OLD(Ray r, vec3 currentColor, float maxDepth) {
    float marchDistance = 0.0;
    vec4 accumulated = vec4(0.0);
   
    vec3 sunDirection = normalize(-dirLights[0].direction);
   
    while(marchDistance < MAX_RANGE && accumulated.a < 0.95) {
        float dist = CubeSDF(r.pos);
       
        // Sphere trace when far from volume
        if(dist > 2.0) {
            float step = dist * 0.5;
            r.pos = r.pos + r.dir * step;
            marchDistance += step;
            continue;
        }
       
        // Volume sampling
        r.pos = r.pos + r.dir * dt;
        marchDistance += dt;
       
        // Check depth
        if(marchDistance > maxDepth) {
            break;
        }
       
        // Soft boundary falloff with warping
        float noiseSize = 0.5;
        vec3 warpOffset = vec3(
            noise((r.pos ) + vec3(time,0,0)),
            noise((r.pos ) + vec3(100.0) + vec3(time,0,0)),
            noise((r.pos ) + vec3(200.0)  + vec3(time,0,0))
        ) * 2.0; // Warp strength
        
        vec3 warpedPos = r.pos + warpOffset;
        float sphereDist = CubeSDF(warpedPos);
        float boundaryFalloff = smoothstep(1.0, -1.0, sphereDist)*1.0;
       

        float density = noise((r.pos * noiseSize) + vec3(0, 0, time)) * boundaryFalloff;
       
        // Only process if density is positive
        if(density > 0.0) {
            // Directional derivative for lighting
            float stepSize = 0.3;
            vec3 lightSamplePos = r.pos + sunDirection * stepSize;
            
            // Apply the SAME warping to the light sample position

            
            vec3 warpedLightPos = lightSamplePos + warpOffset;
            float lightBoundaryFalloff = smoothstep(2.0, -1.0, CubeSDF(warpedLightPos));
            
            // Apply the same time offset to the light sample
            float densityAtLight = noise((lightSamplePos * noiseSize) + vec3(0, 0, time))
                                 * lightBoundaryFalloff;
            
            float diffuse = clamp((density - densityAtLight) / stepSize, 0.0, 1.0) * 2.0;
           
            // Color mixing - ambient + sun
            vec3 ambient = vec3(0.7, 0.7, 0.7) * 1.1;
            vec3 sunColor = vec3(1.0, 0.9, 0.7) * 0.8;
            vec3 lighting = ambient + sunColor * diffuse;
            vec3 cloudColor = mix(vec3(1.0, 1.0, 1.0), vec3(0.4, 0.4, 0.4), density);
            vec4 color = vec4(cloudColor * lighting, density * 0.4);
            color.rgb *= color.a;
            accumulated += color * (1.0 - accumulated.a);
        }
    }
   
    // Blend with background
    return mix(currentColor, accumulated.rgb, accumulated.a);
}

*/
vec3 worldToCubeMap(vec3 pos){
    vec3 cubeMin = volumeCenter - 0.5 * volumeDimension;
    vec3 cubeMax = volumeCenter + 0.5 * volumeDimension;
    vec3 uvw = (pos - cubeMin) / (cubeMax - cubeMin);
    return clamp(uvw, 0.001, 0.999); // Avoid exact boundaries
}

vec3 getDensityGradient(vec3 pos) {
    float eps = dt * 0.5;
    float dx = texture(volumeDensity, worldToCubeMap(pos + vec3(eps, 0, 0))).r 
             - texture(volumeDensity, worldToCubeMap(pos - vec3(eps, 0, 0))).r;
    float dy = texture(volumeDensity, worldToCubeMap(pos + vec3(0, eps, 0))).r 
             - texture(volumeDensity, worldToCubeMap(pos - vec3(0, eps, 0))).r;
    float dz = texture(volumeDensity, worldToCubeMap(pos + vec3(0, 0, eps))).r 
             - texture(volumeDensity, worldToCubeMap(pos - vec3(0, 0, eps))).r;
    return vec3(dx, dy, dz) / (2.0 * eps);
}

bool willHitFluid(Ray r, vec3 boxMin, vec3 boxMax) {
    vec3 invDir = 1.0 / r.dir;
    vec3 t0 = (boxMin - r.pos) * invDir;
    vec3 t1 = (boxMax - r.pos) * invDir;
    
    vec3 tSmaller = min(t0, t1);
    vec3 tBigger = max(t0, t1);
    
    float tMin = max(max(tSmaller.x, tSmaller.y), tSmaller.z);
    float tMax = min(min(tBigger.x, tBigger.y), tBigger.z);
    
    return tMax >= tMin && tMax >= 0.0;
}


bool RayBoxIntersect(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax, out float tNear, out float tFar)
{
    vec3 invDir = 1.0 / rayDir;

    vec3 t0s = (boxMin - rayOrigin) * invDir;
    vec3 t1s = (boxMax - rayOrigin) * invDir;

    vec3 tsmaller = min(t0s, t1s);
    vec3 tbigger  = max(t0s, t1s);

    tNear = max(max(tsmaller.x, tsmaller.y), tsmaller.z);
    tFar  = min(min(tbigger.x, tbigger.y), tbigger.z);

    return tFar >= max(tNear, 0.0);
}


 // You need this


bool isInsideFluid(vec3 pos) {
    // Convert to [0,1] UVW and check bounds
    vec3 uvw = worldToCubeMap(pos);
    bool insideBounds = all(greaterThanEqual(uvw, vec3(0.0))) && all(lessThanEqual(uvw, vec3(1.0)));

    if (!insideBounds)
        return false;

    float d = texture(volumeDensity, uvw).r;
    return d > 0.01; // threshold for "non-air"
}

Hit findNextSurface(vec3 origin, vec3 dir, bool findNextFluidEntry, float maxDist)
{
    Hit info;
    info.found = false; // Initialize properly
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
    origin += dir * (tNear + 0.01); // Small epsilon

    bool hasExitedFluid = !isInsideFluid(origin);
    bool hasEnteredFluid = isInsideFluid(origin);
    vec3 lastPosInFluid = origin;

    float tEnd = min(tFar - tNear, maxDist);

    for (float t = 0.0; t < tEnd; t += stepSize)
    {
        vec3 pos = origin + dir * t;
        vec3 uvw = worldToCubeMap(pos);
        
        // Ensure we're in valid texture space
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
            if(length(grad) > 0.001) { // Only normalize if gradient is significant
                info.normal = -normalize(grad);
            } else {
                // Fallback to ray direction if gradient is too small
                info.normal = -normalize(dir);
            }
            return info;
        }
    }

    return info;
}







//marches up util wwe hit some object or max range to the skybox
//returns the color we hit. Uses the screen texture 




// Convert world position to screen space (NDC then to [0,1]) (uv map)
vec3 worldToScreen(vec3 world, mat4 view, mat4 proj) {
    vec4 clip = proj * view * vec4(world, 1.0);
    vec3 ndc = clip.xyz / clip.w;
    return ndc * 0.5 + 0.5; 
}

// Convert screen space back to world space using precomputed inverse matrices
vec3 screenToWorld(vec3 screen) {
    // Convert from [0,1] to [-1,1] NDC
    vec3 ndc = screen * 2.0 - 1.0;
    
    // Unproject
    vec4 clip = vec4(ndc, 1.0);
    vec4 viewPos = inverseProjectionMat * clip;
    viewPos /= viewPos.w;
    
    vec4 worldPos = inverseViewMat * viewPos;
    return worldPos.xyz;
}

//wwe need a function to sample the screen tex given a direction
//we have the depth buffer so we can check it for a colision

float densityAlongDirection(vec3 pos, vec3 dir){
    vec3 cubeMin = volumeCenter - 0.5 * volumeDimension;
    vec3 cubeMax = volumeCenter + 0.5 * volumeDimension;
    
    float tNear, tFar;
    bool hit = RayBoxIntersect(pos, dir, cubeMin, cubeMax, tNear, tFar);
    
    // If ray doesn't hit volume or is behind us, return 0
    if (!hit || tFar < 0.0) {
        return 0.0;
    }
    
    // Start from entry point (or current pos if already inside)
    tNear = max(tNear, 0.0);
    float marchDist = tNear;
    float maxDist = min(tFar, MAX_RANGE);
    
    vec3 marchPos = pos + dir * tNear;
    float density = 0.0;
    
    while(marchDist < maxDist){
        marchPos += dir * dt;
        marchDist += dt;
        
        // Only sample if inside volume
        vec3 uvw = worldToCubeMap(marchPos);
        if (all(greaterThanEqual(uvw, vec3(0.0))) && all(lessThanEqual(uvw, vec3(1.0)))) {
            density += texture(volumeDensity, uvw).r * dt;
        }
    }
    return density;
}

vec3 getLightInFluid(vec3 pos, vec3 dir)
{
    // Convert world position to screen space (non-linear depth in [0,1])
    vec3 screenPos = worldToScreen(pos, viewMat, projectionMat);
    float depthAtPos = texture(screenDepth, screenPos.xy).r;

    // If depth buffer sample is clearly in front of our position -> we're occluded.
    // But do not return black; sample the scene color instead.
    // Use a small margin to account for precision differences.
    const float initialMargin = 0.005; // tune between 0.002..0.02

    if (screenPos.z > depthAtPos + initialMargin) {
        // The camera sees geometry closer than 'pos'. Return that opaque color.
        // Note: we don't linearize here because both screenPos.z and depthAtPos
        // are the same non-linear depth [0..1] convention from worldToScreen.
        // Sample scene color safely clamped to screen bounds.
        if (screenPos.x >= 0.0 && screenPos.x <= 1.0 && screenPos.y >= 0.0 && screenPos.y <= 1.0) {
            return texture(screenTexture, screenPos.xy).rgb * dirLights[0].color;
        } else {
            return texture(skybox, dir).rgb * dirLights[0].color;
        }
    }

    // march in world-space along dir and compare with scene depth at each step
    vec3 rayPos = pos;
    float traveled = 0.0;
    const float marchStep = dt;
    const float maxSteps = MAX_RANGE / dt;
    const float depthMargin = 0.01; // tolerance when comparing depths; tune as needed

    for (int i = 0; i < int(maxSteps); ++i)
    {
        rayPos += dir * marchStep;
        traveled += marchStep;

        // Project the current ray position into screen space
        vec3 scr = worldToScreen(rayPos, viewMat, projectionMat);

        // Discard if out of screen bounds
        if (scr.x < 0.0 || scr.x > 1.0 || scr.y < 0.0 || scr.y > 1.0) {
            // continue marching until we hit skybox
            if (traveled > MAX_RANGE) break;
            continue;
        }

        // Read the scene depth at this pixel (non-linear [0..1])
        float sceneDepth = texture(screenDepth, scr.xy).r;

        // If our sample depth scr.z is *behind or at* the scene depth (within tolerance)
        // then we've hit opaque geometry — return the scene color sampled at this pixel.
        // Use a margin because of projection differences and depth precision.
        if (scr.z >= sceneDepth - depthMargin) {
            vec3 color = texture(screenTexture, scr.xy).rgb;
            return color * dirLights[0].color;
        }

        if (traveled > MAX_RANGE) break;
    }

    // No opaque hit found — return skybox sample
    return texture(skybox, dir).rgb * dirLights[0].color;
}



float getReflectance(vec3 incidence, vec3 normal, float n1, float n2) {
    float refractionIndex = n1 / n2;
    float cosIn = -dot(incidence, normal);
    float sinSqrt = refractionIndex * refractionIndex * (1.0 - cosIn * cosIn);
    if(sinSqrt >= 1.0) { return 1.0; } // Total internal reflection
    float cosRefract = sqrt(1.0 - sinSqrt);
    float sqrtRayPerp = (n1 * cosIn - n2 * cosRefract) / (n1 * cosIn + n2 * cosRefract);
    float sqrtRayParal = (n2 * cosIn - n1 * cosRefract) / (n2 * cosIn + n1 * cosRefract);
    return (sqrtRayPerp * sqrtRayPerp + sqrtRayParal * sqrtRayParal) / 2.0;
}






//things which are going wrong
//black regions (neither refraction or reflection)
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
   
    const int MAX_REFRACTIONS = 2;
    
    for(int i = 0; i < MAX_REFRACTIONS; i++){
        // Search for next surface boundary
        bool searchForFluidEntry = !travellingThroughFluid;
        Hit surfaceInfo = findNextSurface(r.pos, r.dir, searchForFluidEntry, 50.0);
        if(!surfaceInfo.found || length(surfaceInfo.normal) < 0.1 && i == 0){ //no fluid hit on enter
            return sceneColor;
        }
        
        if(!surfaceInfo.found || length(surfaceInfo.normal) < 0.1){ //we hit something then we bounced off
            break;
        }

        
        // Attenuate transmittance by density along this segment
        vec3 segmentTransmittance = exp(-surfaceInfo.totalDensity * scatteringCoefficient);
        transmittance *= segmentTransmittance;
        
        // Move to surface with small offset
        r.pos = surfaceInfo.pos;
        vec3 normal = surfaceInfo.normal;
        
        // Ensure normal points against ray direction
        if(dot(normal, r.dir) > 0.0) {
            normal = -normal;
        }
        
        // Calculate indices of refraction
        float iorA = travellingThroughFluid ? 1.33 : 1.0;
        float iorB = travellingThroughFluid ? 1.0 : 1.33;
        float eta = iorA / iorB;
        
        // Calculate reflection and refraction directions
        vec3 refractDir = refract(normalize(r.dir), normal, eta);
        vec3 reflectDir = reflect(normalize(r.dir), normal);
        
        // Calculate fresnel reflectance
        float reflectance = getReflectance(r.dir, normal, iorA, iorB);
        float refractWeight = 1.0 - reflectance;
        
        // Handle total internal reflection
        if(reflectance >= 0.99 || length(refractDir) < 0.1) {
            refractDir = reflectDir;
            refractWeight = 0.0;
            reflectance = 1.0;
        }
        
        // Calculate density along each potential path
        float densityAlongRefract = densityAlongDirection(r.pos, refractDir);
        float densityAlongReflect = densityAlongDirection(r.pos, reflectDir);
        
        // Weight density by fresnel terms to choose more important path
        float refractImportance = densityAlongRefract * refractWeight;
        float reflectImportance = densityAlongReflect * reflectance;
        
        bool traceRefractedRay = refractImportance > reflectImportance;
        
        // Update whether we're in fluid based on chosen path
        if(traceRefractedRay) {
            travellingThroughFluid = !travellingThroughFluid;
        }
        
        // Approximate the less important path by sampling environment
        vec3 lessImportantDir = traceRefractedRay ? reflectDir : refractDir;
        float lessImportantWeight = traceRefractedRay ? reflectance : refractWeight;
        float lessImportantDensity = traceRefractedRay ? densityAlongReflect : densityAlongRefract;
        vec3 lessImportantTrans = exp(-lessImportantDensity * scatteringCoefficient);
        
        vec3 envLight = getLightInFluid(r.pos, lessImportantDir);
        accumulatedLight += envLight * transmittance * lessImportantTrans * lessImportantWeight;
        
        // Continue with more important path
        r.dir = traceRefractedRay ? refractDir : reflectDir;
        r.pos += r.dir * 0.02; // Small offset to avoid self-intersection
        transmittance *= (traceRefractedRay ? refractWeight : reflectance);
        
        // Early exit if transmittance is too low
        if(max(transmittance.r, max(transmittance.g, transmittance.b)) < 0.01) {
            break;
        }
    }
    
    // Sample environment for final ray direction
    float remainingDensity = densityAlongDirection(r.pos, r.dir);
    vec3 remainingTransmittance = exp(-remainingDensity * scatteringCoefficient);
    vec3 finalEnvLight = getLightInFluid(r.pos, r.dir);//the final ray direction sample that should return the enviroment
    accumulatedLight += finalEnvLight * transmittance * remainingTransmittance;
    
    return accumulatedLight;
}

//calculates the density up until the end of the volume
//if not on the inside, it just returns 0.0


float linearizeDepth(float depthNDC,float near,float far)
{
    // Convert [0,1] depth to [-1,1] NDC
    float z = depthNDC * 2.0 - 1.0;

    // Perspective linearization
    return (2.0 * near * far) / (far + near - z * (far - near));
}

//we need to manually make the depth testing oin the getiNTERSECTGION
//if we hit somethin in the depth buffer, we just drop this
void main() {
    vec4 ndc = vec4(TexCoords * 2.0 - 1.0, -1.0, 1.0);
    vec4 viewPos = inverseProjectionMat * ndc;        
    viewPos /= viewPos.w;
    vec3 rayDir = normalize((inverseViewMat * vec4(viewPos.xyz, 0.0)).xyz);
    
    Ray r;
    r.pos = cameraPos;
    r.dir = rayDir;


    // Get linearized depth from depth buffer
    float depthSample = texture(screenDepth, TexCoords).r;
    float linearDepth = linearizeDepth(depthSample, nearFar.x, nearFar.y);
    
    // We need the view direction in view space to get the z-component
    vec3 viewDir = normalize(viewPos.xyz); //the ray dir in view sace
    float maxRayDistance = linearDepth / abs(viewDir.z); //some trig here, our secant is the max ray distance, (1/cos(theta)), but view isshe cos (use the diagran because this is very confusoin)
                                                         //also abs because openGL -z is the foward
    vec3 sceneColor = texture(screenTexture, TexCoords).rgb;
   
    // Raymarch with depth limit
    vec3 finalColor = march(r,sceneColor);
   
    FragColor = vec4(finalColor, 1.0);

    
}