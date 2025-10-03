#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "Definitions.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

class Transform
{
private:
    vec3 position;
    vec3 scale;
    glm::quat rotation;

    mat4 transformMat;
    mat3 normalMat;

    bool needsUpdate = true;

    vec3 forward;
    vec3 right;
    vec3 up;

    void updateVectors();
    void updateMatrixes();

    
    
public:
    Transform();

    void setPosition(vec3 pos);
    vec3 getPosition();
    void offSetPosition(vec3 offset);

    //moves the object along its local axis by amount
    void move(vec3 amount);
    void setScale(vec3 sc);
    vec3 getScale();
    void offSetScale(vec3 offset);

    void setRotation(vec3 angles);
    void rotateLocal(vec3 angles);
    void rotateGlobal(vec3 angles);
    void lookAt(vec3 target);

    vec3 getRotation();

    vec3 getForward();
    vec3 getRight();
    vec3 getUp();
    vec3 getTarget();






    
    mat4 getTransformMat(); 
    mat3 getNormalMat();


};

#endif


