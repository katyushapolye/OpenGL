#include "../headers/Light.h"


// PointLight implementations
PointLight::PointLight(Transform transform, vec3 color, float intensity, float radius)
{
    this->transform = transform;
    this->color = color;
    this->intensity = intensity;
    this->type = LightType::POINT;
    this->radius = radius;
}

PointLight::PointLight(vec3 position, vec3 color, float intensity, float radius)
{
    this->transform = Transform();
    this->transform.setPosition(position);
    this->color = color;
    this->intensity = intensity;
    this->type = LightType::POINT;
    this->radius = radius;
}

mat4 PointLight::getViewMatrix(){

    //we will set these values more carefully soon
    return glm::lookAt(
    this->transform.getPosition(),      
    this->transform.getPosition() + this->transform.getForward(),      
    this->transform.getUp()         
    );
    
}

mat4 PointLight::getProjectionMatrix(){

    return glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 1.0f, 100.0f); //big box for directional light

    
}

// SpotLight implementations
SpotLight::SpotLight(Transform transform, vec3 color, float intensity, 
                     float theta, float outerTheta, float radius)
{
    this->transform = transform;
    this->color = color;
    this->intensity = intensity;
    this->type = LightType::SPOT;
    this->theta = theta;
    this->outerTheta = outerTheta;
    this->radius = radius;
}

SpotLight::SpotLight(vec3 position, vec3 direction, vec3 color, float intensity, 
                     float theta, float outerTheta, float radius)
{
    this->transform = Transform();
    this->transform.setPosition(position);
    
    this->color = color;
    this->intensity = intensity;
    this->type = LightType::SPOT;
    this->theta = theta;
    this->outerTheta = outerTheta;
    this->radius = radius;
}

mat4 SpotLight::getViewMatrix(){

    //we will set these values more carefully soon
    return glm::lookAt(
        this->transform.getPosition(),      
        this->transform.getPosition() + vec3(this->transform.getForward().x,this->transform.getForward().y,this->transform.getForward().z), // Negated!     
        this->transform.getUp()         
    );
    
}

mat4 SpotLight::getProjectionMatrix(){

    return glm::perspective(glm::radians((theta*2.0f)),1.0f,1.0f,10.0f); //doesnt work when

    
    
}

// DirectionalLight implementations
DirectionalLight::DirectionalLight(vec3 rotation, vec3 color, float intensity)
{
    this->transform = Transform();
    this->transform.setRotation(rotation); // will add a lookat method later

    this->color = color;
    this->intensity = intensity;
    this->type = LightType::DIRECTIONAL;
}

DirectionalLight::DirectionalLight(Transform transform, vec3 color, float intensity)
{
    this->transform = transform;
    this->color = color;
    this->intensity = intensity;
    this->type = LightType::DIRECTIONAL;
}

mat4 DirectionalLight::getViewMatrix(){
    // Now forward points away from the target (OpenGL convention)
    // So we need to NEGATE it to look in the correct direction
    return glm::lookAt(
        this->transform.getPosition(),      
        this->transform.getPosition() + vec3(this->transform.getForward().x,this->transform.getForward().y,this->transform.getForward().z), // Negated!     
        this->transform.getUp()         
    );
}

mat4 DirectionalLight::getProjectionMatrix(){

    return glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 1.0f, 30.0f); //big box for directional light, with the light at its center, unconvential 

    
}