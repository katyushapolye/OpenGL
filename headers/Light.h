#ifndef LIGHT_H
#define LIGHT_H
#include "Definitions.h"
#include "Transform.h"
class Light
{
private:

    
public:
    Transform transform;
    vec3 color;
    float intensity;
    LightType type;
    float theta; //for spot light

    Light(Transform transform,LightType type, vec3 color = vec3(1.0f,1.0f,1.0f), float intensity = 1.0f, float theta = 12.5f);
    Light(LightType type, vec3 color = vec3(1.0f,1.0f,1.0f), float intensity = 1.0f);



};

#endif


