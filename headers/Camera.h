#ifndef CAMERA_H
#define CAMERA_H

#include "Definitions.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
class Camera
{
private:
 

    vec3 position;
    vec3 rotation;

    float fov;
    float aspectRatio;

    float nearPlane;
    float farPlane;

    unsigned int width;
    unsigned int height;

    vec3 target; //what is the target for the camera

    vec3 right; //relative right of the camera
    vec3 forward; //where the camera is pointing at
    vec3 up; //relative up of the camera

    
    
public:
    Camera(float fov, float aspectRatio, float nearPlane, float farPlane, unsigned int width, unsigned int height);

    void setPosition(vec3 pos);
    vec3 getPosition();

    void setRotation(vec3 rot);
    vec3 getRotation();

    void setTarget(vec3 tgt);
    vec3 getTarget();

    vec3 getForward();
    vec3 getUp();

    mat4 getViewMat();
    mat4 getProjectionMat();
    
    void receiveInput(vec2 inputDir,vec2 mouseDir, float deltaTime, bool zoomIn, bool zoomOut);
};



#endif