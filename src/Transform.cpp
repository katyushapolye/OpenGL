#include "../headers/Transform.h"

//implement rotation as quaterions
//local rotation and global rotation
Transform::Transform() {
    this->position = vec3(0.0f,0.0f,0.0f);
    this->scale = vec3(1.0f,1.0f,1.0f);
    this->rotation = vec3(0.0f,0.0f,0.0f);
    this->transformMat = mat4(1.0f); //indentity mat

    this->up = vec3(0.0f,1.0f,0.0f);
    this->forward = vec3(0.0f,0.0f,1.0f); 
    this->right = vec3(1.0f,0.0f,0.0f);
}

//creates and caches the transform matrix
mat4 Transform::getTransformMat() {
    if(!(this->needsUpdate)){
        return this->transformMat;
    }

    //update the transformation matrix and caches it
    this->transformMat = glm::translate(glm::identity<mat4>(), this->position);
    //the rotation is applied in order of x y and z, leave quartenio
    //quartenion
    this->transformMat *= glm::mat4_cast(this->rotation);

    this->transformMat = glm::scale(this->transformMat, this->scale);

    this->needsUpdate = false;
    return this->transformMat;


}


void Transform::setPosition(vec3 pos) {
    this->position = pos;
    this->needsUpdate = true;
}

vec3 Transform::getPosition() {
    return this->position;

}

void Transform::offSetPosition(vec3 offset) {
    this->position += offset;
     this->needsUpdate = true;    
}

void Transform::setScale(vec3 sc) {
    this->scale = sc;
     this->needsUpdate = true;
}

vec3 Transform::getScale() {
    return this->scale;
}

void Transform::offSetScale(vec3 offset) {
    this->scale += offset;
     this->needsUpdate = true;
}
//Sets therotation in degrees
void Transform::setRotation(vec3 rot) {
    this->rotation = glm::quat(glm::radians(rot));
    this->needsUpdate = true; 
    updateVectors();

}

vec3 Transform::getRotation() {
    return glm::degrees(glm::eulerAngles(this->rotation));

}

//i dont have any idea how quaternions work
void Transform::rotateGlobal(vec3 angles) {

    glm::quat deltaRot = glm::quat(glm::radians(angles));
    
    // For global rotation: delta rotation first
    this->rotation = glm::normalize(deltaRot * this->rotation);
    
    this->needsUpdate = true;
    updateVectors();
}

void Transform::rotateLocal(vec3 angles) { 
    glm::quat deltaRot = glm::quat(glm::radians(angles));
    
    // For local rotation: delta rotation last
    this->rotation = glm::normalize(this->rotation * deltaRot);
    
    this->needsUpdate = true;
    updateVectors();
}

vec3 Transform::getForward() {
    return this->forward;
}
vec3 Transform::getRight() {
    return this->right;
}
vec3 Transform::getUp() {
    return this->up;
}

void Transform::updateVectors() {
    //i could optimize this but i understand how it is

    // Use the quaternion to directly transform the basis vectors
    // World forward vector (0, 0, -1) transformed by quaternion
    this->forward = glm::normalize(this->rotation * vec3(0.0f, 0.0f, 1.0f));
    
    // World up vector (0, 1, 0) transformed by quaternion
    vec3 worldUp = glm::normalize(this->rotation * vec3(0.0f, 1.0f, 0.0f));
    
    // Recalculate right using the actual forward and world up
    this->right = glm::normalize(glm::cross(this->forward, worldUp));
    
    // Recalculate up using right and forward (orthonormal basis)
    this->up = glm::normalize(glm::cross(this->right, this->forward));


}