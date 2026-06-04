#version 420 core

struct Material {
    vec3 diffuseColor;

    sampler2D diffuseMap; //TEX UNITY 1
    sampler2D specularMap; //TEX UNITY 2
    sampler2D reflectionMap; //TEX UNITY 3
    sampler2D normalMap; //TEX UNITY 4
    float shininess;
};
struct PointLight {
    vec3 position; //16 bytes
    vec3 color; //16 bytes
    float intensity; //4 bytes
    float radius; //4 bytes
};

struct DirectionalLight {

    vec3 direction;
    float intensity;
    vec3 color;

};

struct SpotLight {

    vec3 position;
    vec3 direction;
    float intensity;
    vec3 color;
    float theta;

};

//Ins an outs
in vec2 fTexCoord;
in vec3 fVertexNormal;
in vec3 fVertexNormal2;
in vec3 fFragPos;


out vec4 FragColor;



layout(std140, binding = 1) uniform Camera  {
    vec3 cameraPos;
    vec3 cameraRot;
    vec2 nearFar;
};

layout(std140, binding = 2) uniform PointLights_UBO{
    int pointLightCount;
    PointLight pointLights_UBO[16];//big

};


//Lighting, we need a attr to not iterate over the whole arrray (light count)
#define NR_POINT_LIGHTS 16
uniform PointLight pointLights[NR_POINT_LIGHTS];


#define NR_DIRECTIONAL_LIGHTS 4
uniform DirectionalLight dirLights[NR_DIRECTIONAL_LIGHTS];

#define NR_SPOT_LIGHTS 16
uniform SpotLight spotLights[NR_SPOT_LIGHTS];

uniform vec3 ambientLight;

//Skybox for enviroment mapping

uniform samplerCube skybox; //TEX UNITY 0

//Materials

uniform Material material0;


//Camera

//shadows

in vec4 fLightFragPos[16];
uniform sampler2DArray shadowMaps;
uniform float shadowMapSize;
uniform int directionalLightCount;
uniform int spotLightCount;


vec3 shadowTest(vec4[16] lightFragPos, vec3 normal)
//no pcf because i bugged the fucking thing
{
    float totalShadowDirLight = 0.0;
    int activeDirLights = 0;
    vec2 texelSize = vec2(1.0 / shadowMapSize,1.0/shadowMapSize);
   
    // first we do a dirlight pass
    for(int lightIdx = 0; lightIdx < directionalLightCount; lightIdx++) {
        vec3 lightDir = normalize(-dirLights[lightIdx].direction);
        vec3 projCoords = lightFragPos[lightIdx].xyz / lightFragPos[lightIdx].w;
        projCoords = projCoords * 0.5 + 0.5;
       
        if (projCoords.z < 0.0 || projCoords.z > 1.0) {
            continue;
        }
       
        float currentDepth = projCoords.z;
        float bias = max(0.001 * (1.0 - dot(normal, lightDir)), 0.0005);
       
        //float pcfDepth = texture(shadowMaps, vec3(projCoords.xy, lightIdx)).r;
        //float lightShadow = (currentDepth - bias > pcfDepth ? 1.0 : 0.0);
        float shadow = 0.0;

        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMaps, vec3(projCoords.xy + vec2(x, y) * texelSize,lightIdx)).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;
       
        totalShadowDirLight += shadow;
        activeDirLights++;
    }
   
    float totalShadowSpotLight = 0.0;
    int activeSpotLights = 0;
   
    // then we offset the Idx and process spotlight
    for(int lightIdx = 0; lightIdx < spotLightCount; lightIdx++) {
        vec3 lightDir = normalize(-spotLights[lightIdx].direction);
        int fragPosIdx = directionalLightCount + lightIdx;
        vec3 projCoords = lightFragPos[fragPosIdx].xyz / lightFragPos[fragPosIdx].w;
        projCoords = projCoords * 0.5 + 0.5;
       
        if (projCoords.z < 0.0 || projCoords.z > 1.0) {
            continue;
        }
       
        float currentDepth = projCoords.z;
        float bias = max(0.001 * (1.0 - dot(normal, lightDir)), 0.0005);
       
        //float pcfDepth = texture(shadowMaps, vec3(projCoords.xy, fragPosIdx)).r;
        //float lightShadow = (currentDepth - bias > pcfDepth ? 1.0 : 0.0);
                float shadow = 0.0;

        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMaps, vec3(projCoords.xy + vec2(x, y) * texelSize,fragPosIdx)).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;
       
        totalShadowSpotLight += shadow;
        activeSpotLights++;
    }
   
    float dirShadow = activeDirLights > 0 ? totalShadowDirLight / float(activeDirLights) : 0.0;
    float spotShadow = activeSpotLights > 0 ? totalShadowSpotLight / float(activeSpotLights) : 0.0;
    return vec3(dirShadow, spotShadow, 0);
}

 
vec3 ambientLightPass(){
    return ambientLight;
}

vec3 pointLightPass(vec3 viewDir,vec3 normal){
    vec3 pointContribution = vec3(0.0f,0.0f,0.0f);
    for (int i = 0; i < NR_POINT_LIGHTS; i++){
        //difuse and ambient component
        vec3 lightDir = normalize(pointLights[i].position - fFragPos);
        //inverse square law
        float distance = length(pointLights[i].position - fFragPos);
        float attenuation = pointLights[i].intensity / (1.0 + distance * distance);

        vec3 diffuseColor = max(dot(normal,lightDir),0.0f) * pointLights[i].color;
        //specular
        vec3 reflectDir = reflect(-lightDir,normal);
        float specFactor = material0.shininess >= 0.05f? pow(max(dot(viewDir,reflectDir),0.0),material0.shininess*64) : 0.0;
        vec3 specularColor =  specFactor * pointLights[i].color * texture(material0.specularMap,fTexCoord).rgb;
        pointContribution  = pointContribution + (diffuseColor + specularColor)* attenuation;

    }
    return pointContribution;
}
 


vec3 directionalLightPass(vec3 viewDir,vec3 normal){
    vec3 dirContribution = vec3(0.0f,0.0f,0.0f);
    for (int i = 0; i < NR_DIRECTIONAL_LIGHTS; i++){
        vec3 lightDir = normalize(-dirLights[i].direction);
        
        vec3 diffuseColor = max(dot(normal,lightDir),0.0f) * dirLights[i].color;
        vec3 reflectDir = reflect(-lightDir,normal);
        float specFactor = material0.shininess >= 0.05f? pow(max(dot(viewDir,reflectDir),0.0),material0.shininess*32) : 0.0;
        float height = fTexCoord.y; //heigh is inverted in gl for reasonsunknon
        vec3 top = vec3(1.0,1.0,1.0);
        vec3 botton = vec3(0.5,0.5,0.5);
        vec3 color = mix(botton,top,height);
        vec3 specularColor = specFactor * dirLights[i].color * color;

       
        // Remove the extra color multiplication
        dirContribution = dirContribution + (diffuseColor + specularColor) * dirLights[i].intensity;
    }
    return dirContribution;
}

vec3 spotLightPass(vec3 viewDir,vec3 normal){
    vec3 spotcontribution = vec3(0.0f,0.0f,0.0f);
    for (int i = 0; i < NR_SPOT_LIGHTS; i++){
        vec3 lightDir = normalize(spotLights[i].position - fFragPos); 
        //calculate the angle between foward light and this fragment dir
        float phi = dot(normalize(spotLights[i].direction), -lightDir); //will spit the cos(phi) in radians
        //if this angle is bigger than theta we are out of the cone
        if(phi > cos(radians(spotLights[i].theta))){ //if phi is bigger, 
            //inverse square law
            float distance = length(spotLights[i].position - fFragPos);
            float attenuation = spotLights[i].intensity / (1.0 + distance * distance);

            vec3 diffuseColor = max(dot(normal,lightDir),0.0f) * spotLights[i].color;
            //specular
            vec3 reflectDir = reflect(-lightDir,normal);
            float specFactor = material0.shininess >= 0.05f? pow(max(dot(viewDir,reflectDir),0.0),material0.shininess*32) : 0.0;
            vec3 specularColor =  specFactor * spotLights[i].color * texture(material0.specularMap,fTexCoord).rgb;
            spotcontribution  = spotcontribution + (diffuseColor + specularColor)* attenuation;

        }
        else{
            spotcontribution  = spotcontribution + vec3(0.0f,0.0f,0.0f);
        }

        


    

    }

    return spotcontribution;

}


float hash(float n)
{
    return fract(sin(n) * 43758.5453123);
}




void main()
{
    //insane trick for edge noormals
    //i use the UV coords to check where this frag is in the blade (righ or left)
//
    float width = fTexCoord.x;
    vec3 normal = fVertexNormal; //fallback normal
    if(gl_FrontFacing){
        normal = -normal;

    }
    //control the color now based on height now


float height = 1-fTexCoord.y;

    vec3 bottom = vec3(0.12, 0.15, 0.04); // dark earthy brown
    vec3 top    = vec3(0.51, 0.75/1.4, 0.42/1.7); // dry straw yellow
    vec3 color =  mix(bottom, top, height )    ;
    float randomValue = hash(float(fFragPos));
    //color *= mix(0.9, 1.1, float(randomValue));
    vec3 viewDir = normalize(cameraPos - fFragPos);

    vec3 shadowValue = (1 - shadowTest(fLightFragPos, normal));
    vec3 lightContribution = spotLightPass(viewDir, normal) * shadowValue.y
                       + pointLightPass(viewDir, normal)
                       + directionalLightPass(viewDir, normal) * shadowValue.x
                       + ambientLightPass();
    vec3 objectColor =   lightContribution * color;



    FragColor = vec4(objectColor, texture(material0.diffuseMap,fTexCoord).a);
    //float depth = gl_FragCoord.z; // divide by far for demonstration
    //FragColor = vec4(vec3(depth), 1.0);
}