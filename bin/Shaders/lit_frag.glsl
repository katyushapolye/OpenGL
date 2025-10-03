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



layout(std140, binding = 1) uniform Camera  {
    vec3 cameraPos;
    vec3 cameraRot;
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
{
    float totalShadowDirLight = 0.0;
    
    for(int lightIdx = 0; lightIdx < directionalLightCount; lightIdx++) {
        vec3 lightDir = normalize(-dirLights[lightIdx].direction);
        vec3 projCoords = lightFragPos[lightIdx].xyz / lightFragPos[lightIdx].w;
        projCoords = projCoords * 0.5 + 0.5;
        
        // Skip this light if outside shadow map bounds
        if (projCoords.z < 0.0 || projCoords.z > 1.0) {
            continue; // Behind near plane or beyond far plane
        }
        
        float currentDepth = projCoords.z;
        float bias = max(0.001 * (1.0 - dot(normal, lightDir)), 0.0005);
        
        vec2 texelSize = vec2(1.0 / 4096.0, 1.0 / 4096.0);
        
        // Gaussian kernel weights
        float kernel[16];
        kernel[0] = 0.0232; kernel[1] = 0.0394; kernel[2] = 0.0394; kernel[3] = 0.0232;
        kernel[4] = 0.0394; kernel[5] = 0.0669; kernel[6] = 0.0669; kernel[7] = 0.0394;
        kernel[8] = 0.0394; kernel[9] = 0.0669; kernel[10] = 0.0669; kernel[11] = 0.0394;
        kernel[12] = 0.0232; kernel[13] = 0.0394; kernel[14] = 0.0394; kernel[15] = 0.0232;
        
        // Sample offsets
        vec2 offsets[16];
        offsets[0] = vec2(-1.5, -1.5); offsets[1] = vec2(-0.5, -1.5); offsets[2] = vec2(0.5, -1.5); offsets[3] = vec2(1.5, -1.5);
        offsets[4] = vec2(-1.5, -0.5); offsets[5] = vec2(-0.5, -0.5); offsets[6] = vec2(0.5, -0.5); offsets[7] = vec2(1.5, -0.5);
        offsets[8] = vec2(-1.5,  0.5); offsets[9] = vec2(-0.5,  0.5); offsets[10] = vec2(0.5,  0.5); offsets[11] = vec2(1.5,  0.5);
        offsets[12] = vec2(-1.5,  1.5); offsets[13] = vec2(-0.5,  1.5); offsets[14] = vec2(0.5,  1.5); offsets[15] = vec2(1.5,  1.5);
        
        float lightShadow = 0.0;
        for (int sampleIdx = 0; sampleIdx < 16; sampleIdx++) {
            vec2 offset = offsets[sampleIdx] * texelSize;
            float pcfDepth = texture(shadowMaps, vec3(projCoords.xy + offset, lightIdx)).r;
            lightShadow += (currentDepth - bias > pcfDepth ? 1.0 : 0.0) * kernel[sampleIdx];
        }
        
        totalShadowDirLight += lightShadow;
    }
    
    float totalShadowSpotLight = 0.0;
    for(int lightIdx = 0; lightIdx < spotLightCount; lightIdx++) {
        vec3 lightDir = normalize(-spotLights[lightIdx].direction);
        // FIX: Offset the light fragment position index for spotlights
        int fragPosIdx = directionalLightCount + lightIdx;
        vec3 projCoords = lightFragPos[fragPosIdx].xyz / lightFragPos[fragPosIdx].w;
        projCoords = projCoords * 0.5 + 0.5;
        
        // Skip this light if outside shadow map bounds
        if (projCoords.z < 0.0 || projCoords.z > 1.0) {
            continue; // Behind near plane or beyond far plane
        }
        
        float currentDepth = projCoords.z;
        float bias = max(0.001 * (1.0 - dot(normal, lightDir)), 0.0005);
        
        vec2 texelSize = vec2(1.0 / 4096.0, 1.0 / 4096.0);
        
        // Gaussian kernel weights
        float kernel[16];
        kernel[0] = 0.0232; kernel[1] = 0.0394; kernel[2] = 0.0394; kernel[3] = 0.0232;
        kernel[4] = 0.0394; kernel[5] = 0.0669; kernel[6] = 0.0669; kernel[7] = 0.0394;
        kernel[8] = 0.0394; kernel[9] = 0.0669; kernel[10] = 0.0669; kernel[11] = 0.0394;
        kernel[12] = 0.0232; kernel[13] = 0.0394; kernel[14] = 0.0394; kernel[15] = 0.0232;
        
        // Sample offsets
        vec2 offsets[16];
        offsets[0] = vec2(-1.5, -1.5); offsets[1] = vec2(-0.5, -1.5); offsets[2] = vec2(0.5, -1.5); offsets[3] = vec2(1.5, -1.5);
        offsets[4] = vec2(-1.5, -0.5); offsets[5] = vec2(-0.5, -0.5); offsets[6] = vec2(0.5, -0.5); offsets[7] = vec2(1.5, -0.5);
        offsets[8] = vec2(-1.5,  0.5); offsets[9] = vec2(-0.5,  0.5); offsets[10] = vec2(0.5,  0.5); offsets[11] = vec2(1.5,  0.5);
        offsets[12] = vec2(-1.5,  1.5); offsets[13] = vec2(-0.5,  1.5); offsets[14] = vec2(0.5,  1.5); offsets[15] = vec2(1.5,  1.5);
        
        float lightShadow = 0.0;
        for (int sampleIdx = 0; sampleIdx < 16; sampleIdx++) {
            vec2 offset = offsets[sampleIdx] * texelSize;
            // FIX: Use the offsetted index for shadow map sampling
            float pcfDepth = texture(shadowMaps, vec3(projCoords.xy + offset, fragPosIdx)).r;
            lightShadow += (currentDepth - bias > pcfDepth ? 5.0 : 0.0) * kernel[sampleIdx];
        }
        
        totalShadowSpotLight += lightShadow;
    }
    
    float dirShadow = directionalLightCount > 0 ? totalShadowDirLight / float(directionalLightCount) : 0.0;
    float spotShadow = spotLightCount > 0 ? totalShadowSpotLight / float(spotLightCount) : 0.0;

    return vec3(dirShadow, spotShadow, 0);
}


void main()
{
    

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

    //to debug the shadow, we plot the frag shadow contribution

    //vec3 debug = normal * 0.5 + 0.5;
    //FragColor.rgb = debug;   

}