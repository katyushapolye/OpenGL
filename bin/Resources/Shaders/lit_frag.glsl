#version 440 core

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
    mat4 lightMat;

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
in vec3 fFragPos;
in vec4 fLightFragPos[16];
in mat3 fTBNMat;

out vec4 FragColor;
//layout (depth_greater) out float gl_FragDepth;



layout(std140, binding = 1) uniform Camera  {
    vec3 cameraPos;
    vec3 cameraRot;
    vec2 nearFar;
};




//Lighting, we need a attr to not iterate over the whole arrray (light count)
#define NR_POINT_LIGHTS 16
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform int pointLightCount;


#define NR_DIRECTIONAL_LIGHTS 4
uniform DirectionalLight dirLights[NR_DIRECTIONAL_LIGHTS];
uniform int directionalLightCount;

#define NR_SPOT_LIGHTS 16
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
uniform int spotLightCount;


uniform vec3 ambientLight;

//Skybox for enviroment mapping

uniform samplerCube skybox; //TEX UNIT 0

//Materials (check the tex unit inside them)
uniform Material material0;

//shadow mapping
uniform mat4 lightMatrices[16];
uniform sampler2DArray shadowMaps; //TEX UNIT 15 
uniform float shadowMapSize;
uniform int activeShadowCasters;




vec3 pointLightPass(vec3 viewDir,vec3 normal){
    vec3 pointContribution = vec3(0.0f,0.0f,0.0f);
    for (int i = 0; i < pointLightCount; i++){
        //difuse and ambient component
        vec3 lightDir = normalize(pointLights[i].position - fFragPos);
        //inverse square law
        float distance = length(pointLights[i].position - fFragPos);
        float attenuation = pointLights[i].intensity / (1.0 + distance*distance);

        vec3 diffuseColor = max(dot(normal,lightDir),0.0f) * pointLights[i].color;
        //specular
        vec3 reflectDir = reflect(-lightDir,normal); //we use the halfwat vector for bling phong specular reflections, i really didnt notice MUCH change to it
        vec3 halfwayDir = normalize(lightDir + viewDir);
        //
        float specFactor = material0.shininess >= 0.05f? pow(max(dot(viewDir,halfwayDir),0.0),material0.shininess*32) : 0.0;
        vec3 specularColor =  specFactor * pointLights[i].color * texture(material0.specularMap,fTexCoord).rgb;
        pointContribution  = pointContribution + (diffuseColor + specularColor)* attenuation;

    }
    return pointContribution;
}
 
vec3 ambientLightPass(){
    return pow(ambientLight,vec3(4.4)); //i just made up a magic number for this
}

vec3 directionalLightPass(vec3 viewDir, vec3 normal){
    vec3 dirContribution = vec3(0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < directionalLightCount; i++){
        vec3 lightDir = normalize(-dirLights[i].direction);
        //lightDir.z = -lightDir.z; //because our foward vector in c++ points TOWARDS what im looking, but openGL uses the opposite (-z is the foward vector)
        //i fixed this in c++
        //people that thought about this deserve to be shot
       
        // Diffuse
        vec3 diffuseColor = max(dot(normal, lightDir),0.0) * dirLights[i].color;
        
        // Specular
        
            vec3 reflectDir = reflect(-lightDir,normal); //we use the halfwat vector for bling phong specular reflections, i really didnt notice MUCH change to it
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float specFactor = material0.shininess >= 0.05f? pow(max(dot(viewDir,reflectDir),0.0),material0.shininess*256) : 0.0;
            vec3 specularColor =  specFactor * dirLights[i].color * texture(material0.specularMap,fTexCoord).rgb;
       
        dirContribution += (diffuseColor + specularColor) * dirLights[i].intensity;
    }
    
    return dirContribution;
}

vec3 spotLightPass(vec3 viewDir,vec3 normal){
    vec3 spotcontribution = vec3(0.0f,0.0f,0.0f);
    for (int i = 0; i < spotLightCount; i++){
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
            vec3 reflectDir = reflect(-lightDir,normal); //we use the halfwat vector for bling phong specular reflections, i really didnt notice MUCH change to it
            vec3 halfwayDir = normalize(lightDir + viewDir);
            //
            float specFactor = material0.shininess >= 0.05f? pow(max(dot(viewDir,reflectDir),0.0),material0.shininess*256) : 0.0;
            vec3 specularColor =  specFactor * pointLights[i].color * texture(material0.specularMap,fTexCoord).rgb;
            spotcontribution  = spotcontribution + (diffuseColor + specularColor)* attenuation;

        }
        else{
            spotcontribution  = spotcontribution + vec3(0.0f,0.0f,0.0f);
        }

        


    

    }

    return spotcontribution;

}



//we need one shadow test for each light type so it doesnt cause problems
//and them sum with the others if exist
//each component return a shadow value for the light type

//light frag pos stores this frag position in each of the possible light space (maximum 16 light).
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
void main()
{
    float alpha = texture(material0.diffuseMap,fTexCoord).a;
   // if(alpha < 0.1) {
   //     discard;
   // }
    
    // For alpha between 10 and 255 (assuming normalized 0-1 range)
    // You can't disable depth write directly, but you can:
    
    // Option 1: Write current depth (default behavior)
    // The fragment still writes to depth buffer
    
    // Option 2: Write a far depth value to simulate "no depth write"
    

    
    

    vec3 viewDir = normalize(cameraPos - fFragPos);
    vec3 normal = normalize(fVertexNormal); //fallback normal
    if(textureSize(material0.normalMap, 0).x >1){ //if we have a normal map
        normal =   normalize(fTBNMat * (texture(material0.normalMap,fTexCoord).rgb*2.0 - 1.0)); 
    }
    vec3 shadowValue =  (1-shadowTest(fLightFragPos,normal));
    vec3 lightContribution = spotLightPass(viewDir,normal) *(shadowValue.y) + pointLightPass(viewDir,normal)+ directionalLightPass(viewDir,normal)* (shadowValue.x)  + ambientLightPass();
    vec3 objectColor =   lightContribution * texture(material0.diffuseMap, fTexCoord).rgb;

    //enviroment reflection calculation

        //enviroment reflection calculation
    vec3 I = -viewDir;
    vec3 R = reflect(I,normalize(normal));
    vec3 reflectionColor = (texture(skybox,R).rgb * texture(material0.reflectionMap,fTexCoord).rgb) * lightContribution; //returns all black in non-reflective objects
    

    vec3 finalColor = (reflectionColor + objectColor)*material0.diffuseColor;
    //(1-reflectivity) * (diffuse) + (reflectivity) * reflectionmap + specular

    FragColor.a = texture(material0.diffuseMap,fTexCoord).a;


    FragColor.rgb = pow(vec3(finalColor.rgb),vec3(1/2.2));



}