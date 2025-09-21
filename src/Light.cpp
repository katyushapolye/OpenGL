#include "../headers/Light.h"

Light::Light(Transform transform,LightType type, vec3 color, float intensity,float theta) {
    this->transform = transform;
    this->type = type;
    this->color = color;
    this->intensity = intensity;
    this->theta = theta;
}

Light::Light(LightType type, vec3 color, float intensity) {
    this->transform = Transform();
    this->transform.setPosition(vec3(0.0f,0.0f,0.0f));
    this->transform.setScale(vec3(1.0f,1.0f,1.0f));
    this->transform.setRotation(vec3(0.0f,0.0f,0.0f));
    this->type = type;
    this->color = color;
    this->intensity = intensity;
    this->theta = 12.5f;
}