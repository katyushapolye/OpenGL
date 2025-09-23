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
    this->transform.setRotation(direction); // Assuming Transform has method to set direction
    this->color = color;
    this->intensity = intensity;
    this->type = LightType::SPOT;
    this->theta = theta;
    this->outerTheta = outerTheta;
    this->radius = radius;
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