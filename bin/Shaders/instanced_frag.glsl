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
        float height = 1-fTexCoord.y; //heigh is inverted in gl for reasonsunknon
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



void main()
{
    //insane trick for edge noormals
    //i use the UV coords for 

    float width = fTexCoord.x;
    vec3 normal = normalize(mix(fVertexNormal,fVertexNormal2,width)); //fallback normal
    if(gl_FrontFacing){
        normal = -normal;

    }
    //control the color now based on height now
    float height = 1-fTexCoord.y; //heigh is inverted in gl for reasonsunknon
    vec3 top = vec3(0.05,0.2,0.01);
    vec3 botton = vec3(0.5,0.5,0.1);
    vec3 color = mix(botton,top,height);



    vec3 viewDir = normalize(cameraPos - fFragPos);

    vec3 lightContribution = spotLightPass(viewDir,normal) + pointLightPass(viewDir,normal)+ ambientLightPass()+ directionalLightPass(viewDir,normal);
    vec3 objectColor =   lightContribution * color;



    FragColor = vec4(objectColor, texture(material0.diffuseMap,fTexCoord).a);
    //float depth = gl_FragCoord.z; // divide by far for demonstration
    //FragColor = vec4(vec3(depth), 1.0);
}