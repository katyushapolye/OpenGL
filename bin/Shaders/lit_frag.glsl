#version 330 core
struct Camera{
    vec3 position;
};
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    float shininess;
};
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
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

out vec4 FragColor;
 


//Lighting
#define NR_POINT_LIGHTS 16
uniform PointLight pointLights[NR_POINT_LIGHTS];


#define NR_DIRECTIONAL_LIGHTS 4
uniform DirectionalLight dirLights[NR_DIRECTIONAL_LIGHTS];

#define NR_SPOT_LIGHTS 16
uniform SpotLight spotLights[NR_SPOT_LIGHTS];

uniform vec3 ambientLight;

//Materials

uniform Material material0;

//Camera
uniform Camera camera;

vec3 pointLightPass(vec3 viewDir){
    vec3 pointContribution = vec3(0.0f,0.0f,0.0f);
    for (int i = 0; i < NR_POINT_LIGHTS; i++){
        //difuse and ambient component
        vec3 normal = normalize(fVertexNormal);
        vec3 lightDir = normalize(pointLights[i].position - fFragPos);
        //inverse square law
        float distance = length(pointLights[i].position - fFragPos);
        float attenuation = pointLights[i].intensity / (1.0 + distance * distance);

        vec3 diffuseColor = max(dot(normal,lightDir),0.0f) * pointLights[i].color;
        //specular
        vec3 reflectDir = reflect(-lightDir,normal);
        float specFactor = material0.shininess >= 0.05f? pow(max(dot(viewDir,reflectDir),0.0),material0.shininess*64) : 0.0;
        vec3 specularColor =  specFactor * pointLights[i].color * texture(material0.specular,fTexCoord).rgb;
        pointContribution  = pointContribution + (diffuseColor + specularColor)* attenuation;

    }
    return pointContribution;
}
 

vec3 ambientLightPass(){
    return ambientLight;
}

vec3 directionalLightPass(vec3 viewDir){
    vec3 dirContribution = vec3(0.0f,0.0f,0.0f);
    for (int i = 0; i < NR_DIRECTIONAL_LIGHTS; i++){
        vec3 normal = normalize(fVertexNormal);
        vec3 lightDir = normalize(-dirLights[i].direction);
        
        vec3 diffuseColor = max(dot(normal,lightDir),0.0f) * dirLights[i].color;
        vec3 reflectDir = reflect(-lightDir,normal);
        float specFactor = material0.shininess >= 0.05f? pow(max(dot(viewDir,reflectDir),0.0),material0.shininess*32) : 0.0;
        vec3 specularColor = specFactor * dirLights[i].color * texture(material0.specular,fTexCoord).rgb;
       
        // Remove the extra color multiplication
        dirContribution = dirContribution + (diffuseColor + specularColor) * dirLights[i].intensity;
    }
    return dirContribution;
}

vec3 spotLightPass(vec3 viewDir){
    vec3 spotcontribution = vec3(0.0f,0.0f,0.0f);
    for (int i = 0; i < NR_SPOT_LIGHTS; i++){
        vec3 normal = normalize(fVertexNormal);
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
            vec3 specularColor =  specFactor * spotLights[i].color * texture(material0.specular,fTexCoord).rgb;
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
    vec3 viewDir = normalize(camera.position - fFragPos);
    vec3 objectColor = (spotLightPass(viewDir) + pointLightPass(viewDir)+ ambientLightPass()+ directionalLightPass(viewDir)) * texture(material0.diffuse, fTexCoord).rgb;

    FragColor = vec4(objectColor, texture(material0.diffuse, fTexCoord).a);
    //float depth = gl_FragCoord.z; // divide by far for demonstration
    //FragColor = vec4(vec3(depth), 1.0);
}