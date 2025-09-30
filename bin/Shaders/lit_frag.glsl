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
in vec3 fFragPos;
in vec4 fLightFragPos;
in mat3 fTBNMat;


out vec4 FragColor;



layout(std140, binding = 1) uniform Camera  {
    vec3 cameraPos;
    vec3 cameraRot;
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

uniform samplerCube skybox; //TEX UNIT 0

//Materials (check the tex unit inside them)
uniform Material material0;

//shadow map
uniform sampler2D shadowMap; //TEX UNIT 15 (it decreases)




vec3 pointLightPass(vec3 viewDir,vec3 normal){
    vec3 pointContribution = vec3(0.0f,0.0f,0.0f);
    for (int i = 0; i < NR_POINT_LIGHTS; i++){
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
        float specFactor = material0.shininess >= 0.05f? pow(max(dot(viewDir,reflectDir),0.0),material0.shininess*32) : 0.0;
        vec3 specularColor =  specFactor * pointLights[i].color * texture(material0.specularMap,fTexCoord).rgb;
        pointContribution  = pointContribution + (diffuseColor + specularColor)* attenuation;

    }
    return pointContribution;
}
 
vec3 ambientLightPass(){
    return pow(ambientLight,vec3(4.4)); //i just made up a magic number for this
}

vec3 directionalLightPass(vec3 viewDir,vec3 normal){
    vec3 dirContribution = vec3(0.0f,0.0f,0.0f);
    for (int i = 0; i < NR_DIRECTIONAL_LIGHTS; i++){
        vec3 lightDir = normalize(-dirLights[i].direction);
        
        vec3 diffuseColor = max(dot(normal,lightDir),0.0f) * dirLights[i].color;

        vec3 reflectDir = reflect(-lightDir,normal); //we use the halfwat vector for bling phong specular reflections, i really didnt notice MUCH change to it
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float specFactor = material0.shininess >= 0.05f? pow(max(dot(viewDir,reflectDir),0.0),material0.shininess*256) : 0.0;
        vec3 specularColor =  specFactor * pointLights[i].color * texture(material0.specularMap,fTexCoord).rgb;
       
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



float shadowTest(vec4 lightFragPos,vec3 lightDir,vec3 normal)
{
    vec3 projCoords = lightFragPos.xyz / lightFragPos.w;
    projCoords = projCoords * 0.5 + 0.5;

    // If outside the shadow map, just say "no shadow"
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z > 1.0) {
        return 0.0; // no shadow
    }

    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float bias = max(0.1 * (1.0 - dot(normal, lightDir)), 0.005);  

    //fgaus
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    float kernel[9] = float[](
        0.0625, 0.125, 0.0625,
        0.125,  0.25,  0.125,
        0.0625, 0.125, 0.0625
    );

    // Precomputed offsets for the 3x3 taps
    vec2 offsets[9] = vec2[](
        vec2(-1, -1), vec2( 0, -1), vec2( 1, -1),
        vec2(-1,  0), vec2( 0,  0), vec2( 1,  0),
        vec2(-1,  1), vec2( 0,  1), vec2( 1,  1)
    );

    for (int i = 0; i < 9; i++)
    {
        vec2 offset = offsets[i] * texelSize;
        float pcfDepth = texture(shadowMap, projCoords.xy + offset).r;
        shadow += (currentDepth - bias > pcfDepth ? 1.0 : 0.0) * kernel[i];
    }




    return shadow;
}


void main()
{
    
    vec3 lightDir = normalize(-dirLights[0].direction); //debug
    vec3 viewDir = normalize(cameraPos - fFragPos);
    vec3 normal = normalize(fVertexNormal); //fallback normal
    if(textureSize(material0.normalMap, 0).x >1){ //if we have a normal map
        normal =   normalize(fTBNMat * (texture(material0.normalMap,fTexCoord).rgb*2.0 - 1.0)); 
    }
    vec3 lightContribution = (spotLightPass(viewDir,normal) + pointLightPass(viewDir,normal)+ directionalLightPass(viewDir,normal)) *(1-shadowTest(fLightFragPos,lightDir,fVertexNormal))+ ambientLightPass();
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


}