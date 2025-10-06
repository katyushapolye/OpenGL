#version 440 core
out vec4 FragColor;
  
in vec2 TexCoords;
in mat4 inverseViewMat;
in mat4 inverseProjectionMat;

//screen buffers
uniform sampler2D screenTexture;
uniform sampler2D screenDepth;
uniform float time;

//ray march parameters
#define dt  0.2
#define EPS 0.001
#define MAX_RANGE  50

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

layout(std140, binding = 1) uniform Camera  {
    vec3 cameraPos;
    vec3 cameraRot;
    vec2 nearFar;
};

// Soft sphere falloff instead of hard cube
float softSphereSDF(vec3 pos) {
    vec3 center = vec3(0, 10, 0);
    float radius = 6.0;
    return length(pos - center) - radius;
}

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



// Domain warping for even more organic shapes



vec3 marchRay(Ray r, vec3 currentColor, float maxDepth) {
    float marchDistance = 0.0;
    vec4 accumulated = vec4(0.0);
   
    vec3 sunDirection = normalize(-dirLights[0].direction);
   
    while(marchDistance < MAX_RANGE && accumulated.a < 0.95) {
        float dist = softSphereSDF(r.pos);
       
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
        float sphereDist = softSphereSDF(warpedPos);
        float boundaryFalloff = smoothstep(1.0, -1.0, sphereDist)*1.0;
       

        float density = noise((r.pos * noiseSize) + vec3(0, 0, time)) * boundaryFalloff;
       
        // Only process if density is positive
        if(density > 0.0) {
            // Directional derivative for lighting
            float stepSize = 0.3;
            vec3 lightSamplePos = r.pos + sunDirection * stepSize;
            
            // Apply the SAME warping to the light sample position

            
            vec3 warpedLightPos = lightSamplePos + warpOffset;
            float lightBoundaryFalloff = smoothstep(2.0, -1.0, softSphereSDF(warpedLightPos));
            
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