#include "../headers/Camera.h"

Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane, unsigned int width, unsigned int height)
{
    this->fov = fov;
    this->aspectRatio = aspectRatio;
    this->nearPlane = nearPlane;
    this->farPlane = farPlane;
    this->width = width;
    this->height = height;

    this->position = vec3(0.0f,0.0f,5.0f);
    this->rotation = vec3(0.0f,0.0f,0.0f);
    this->target = vec3(0.0f,0.0f,0.0f);

    this->forward = glm::normalize( this->target - this->position); 
    this->right = glm::normalize(glm::cross(this->forward, vec3(0.0f,1.0f,0.0f)));
    this->up = glm::normalize(glm::cross(this->right, this->forward));





}

void Camera::setPosition(vec3 pos)
{
    this->position = pos;
}
vec3 Camera::getPosition()
{
    return this->position;
}
void Camera::setRotation(vec3 rot)
{
    this->rotation = rot;
}
vec3 Camera::getRotation()
{
    return this->rotation;
}

void Camera::setTarget(vec3 tgt)
{
    this->target = tgt;
}

vec3 Camera::getTarget()
{
    return this->target;
}

//orbital camera control
void Camera::receiveInput(vec2 inputDir, float deltaTime, bool zoomIn, bool zoomOut)
{
    float rotationSpeed = 2.0f * deltaTime;
    float zoomSpeed = 5.0f * deltaTime; // Adjust zoom speed as needed
    
    // Calculate current spherical coordinates
    vec3 toCamera = this->position - this->target;
    float radius = glm::length(toCamera);
    
    // Calculate angles from the normalized direction (independent of radius)
    vec3 normalizedDir = toCamera / radius;
    float currentPitch = asin(normalizedDir.y);
    float currentYaw = atan2(normalizedDir.z, normalizedDir.x);
    
    // Apply rotation input
    currentPitch += inputDir.y * rotationSpeed;
    currentYaw += inputDir.x * rotationSpeed;
   
    // Clamp pitch
    currentPitch = glm::clamp(currentPitch, -1.57f + 0.1f, 1.57f - 0.1f); // ~89 degrees
    
    // Handle zoom input AFTER calculating angles
    if (zoomIn) {
        radius -= zoomSpeed;
    }
    if (zoomOut) {
        radius += zoomSpeed;
    }
    
    // Clamp radius to reasonable bounds
    radius = glm::clamp(radius, 0.5f, 50.0f); // Adjust min/max as needed
   
    this->position = this->target + vec3(
        radius * cos(currentPitch) * cos(currentYaw),
        radius * sin(currentPitch),
        radius * cos(currentPitch) * sin(currentYaw)
    );
   
    this->forward = glm::normalize(this->target - this->position);
    this->right = glm::normalize(glm::cross(this->forward, vec3(0.0f,1.0f,0.0f)));
    this->up = glm::normalize(glm::cross(this->right, this->forward));
}



mat4 Camera::getViewMat()
{
    mat4 view = glm::lookAt(this->position,this->target,this->up);
    return view;
}

mat4 Camera::getProjectionMat()
{
    mat4 proj = glm::perspective(glm::radians(this->fov),(float)this->width/(float)this->height,this->nearPlane,this->farPlane);
    return proj;
}