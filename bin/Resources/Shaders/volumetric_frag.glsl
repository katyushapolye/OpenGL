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
#define dt  0.02
#define EPS 0.001
#define MAX_RANGE  50
//volume parameters
uniform vec3 volumeCenter;
uniform vec3 volumeDimension;
uniform sampler3D volumeDensity;

uniform vec3 scatteringCoefficient;

#define densityMultiplier 1.0

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
    bool fluidToAir;
    vec3 normal;
    

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

vec3 worldToCubeMap(vec3 pos){
    vec3 cubeMin = volumeCenter - 0.5 * volumeDimension;
    vec3 cubeMax = volumeCenter + 0.5 * volumeDimension;

    vec3 uvw = (pos - cubeMin) / (cubeMax - cubeMin);
    return uvw;
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






float densityAlongDirection(vec3 pos, vec3 dir,float dh){
    float density = 0.0;
    float marchDist = 0.0;
    
    // March towards light until we exit the volume
    while(marchDist < MAX_RANGE){
        pos = pos + dir * dh;
        marchDist += dt;
        
        if(CubeSDF(pos) < 0){
            density += texture(volumeDensity, worldToCubeMap(pos)).r * dt * densityMultiplier;
        }
        else{
            break; // Exited volume
        }
    }
    
    return density;
}

vec3 marchRay(Ray r, vec3 currentColor, float maxDepth){
    //add a intersection test here, if no intersection with the cube, just return the current coplor
    vec3 boxMin = volumeCenter - volumeDimension * 0.5;
    vec3 boxMax = volumeCenter + volumeDimension * 0.5;

    float marchDistance = 0.0;
    float volumeDistance = 0.0;

    float rayPathDensity = 0.0;
    vec3 totalLight =  vec3(0.0);

    //ray control
    int refractionCount  =  0;
    bool inFluid = false;


    if (!willHitFluid(r, boxMin, boxMax)) {
        // No intersection with cube, return current color
        vec3 finalTransmittance = exp(-rayPathDensity * scatteringCoefficient);
        vec3 result = totalLight + currentColor * finalTransmittance;
        return pow(clamp(result, 0.0, 1.0), vec3(1.0/2.2));;
    }


   
    while(marchDistance < MAX_RANGE){ 
        //rush to the box
        if(CubeSDF(r.pos) > 1.0){
            r.pos = r.pos + r.dir  * 0.75*CubeSDF(r.pos);
            marchDistance += 0.75*CubeSDF(r.pos);
            continue;
        }
        r.pos = r.pos + r.dir * dt;
        marchDistance += dt;
       

        // Stop at opaque geometry

        if( marchDistance > maxDepth){
           //we hit a non volume object (and not transparent since we dont write transparents to depth buffer)
            
        }
       
       
        if(CubeSDF(r.pos) < 0){

            vec3 gradient = getDensityGradient(r.pos);
            float gradientMagnitude = length(gradient);
            if(gradientMagnitude > 0.1){ //at a interface of either fluid-air or air fluid
                vec3 normal = -normalize(gradient);//our
                

            }




            float stepDensity = texture(volumeDensity,worldToCubeMap(r.pos)).r*dt *  densityMultiplier;
            rayPathDensity += stepDensity;


            float sunlightDensity = densityAlongDirection(r.pos,dirLights[0].direction,0.1);
            vec3 sunlight =  exp(-sunlightDensity*scatteringCoefficient);

            vec3 scatteredLight = sunlight*stepDensity*  scatteringCoefficient;
            vec3 transmitance = exp(-rayPathDensity*scatteringCoefficient);
            totalLight += scatteredLight * transmitance;

        }
    }
    vec3 finalTransmittance = exp(-rayPathDensity * scatteringCoefficient);
    vec3 result = totalLight + currentColor * finalTransmittance;
    return pow(clamp(result, 0.0, 1.0), vec3(1.0/2.2));;
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
    vec3 finalColor = marchRay(r, sceneColor, maxRayDistance);
   
    FragColor = vec4(finalColor, 1.0);

    
}