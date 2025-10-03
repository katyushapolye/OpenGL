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
    //this->target = vec3(0.0f,0.0f,0.0f);

    //this->forward = glm::normalize( this->target - this->position); 
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

vec3 Camera::getForward(){
    return this->forward;
}

vec3 Camera::getUp(){
    return this->up;
}



//orbital camera control
void Camera::receiveInput(vec2 inputDir,vec2 mouseDir, float deltaTime, bool zoomIn, bool zoomOut)
{   
    //first we set the new camera foward vector using the new yaw and pitch
    float mouseSensibility = 0.10;


    float currentYaw  =  this->getRotation().y; //rotation around the y axis

    currentYaw += mouseDir.x*mouseSensibility*deltaTime; //simple calculation
    //same process for pitch
    float currentPitch = this->getRotation().x;

    currentPitch -=  mouseDir.y*mouseSensibility*deltaTime; //inverted ebcause the coorda are inverted
    currentPitch = glm::clamp(currentPitch, -89.0f, 89.0f);

    //foward reccaltulation

    this->forward.x = glm::cos(glm::radians(currentYaw)) * glm::cos(glm::radians(currentPitch));
    this->forward.y = glm::sin(glm::radians(currentPitch));
    this->forward.z = glm::sin(glm::radians(currentYaw)) * cos(glm::radians(currentPitch));

    this->right = glm::normalize(glm::cross(this->forward, vec3(0.0f,1.0f,0.0f)));
    this->up = glm::normalize(glm::cross(this->right, this->forward));

    //printf("%f , %f , %f \n",this->forward.x,this->forward.y,this->forward.z);

    this->setRotation(vec3(currentPitch,currentYaw,this->getRotation().z));


    //now we handle keyboard input
    //we want 

    this->position = this->position +  (10*inputDir.y*this->forward*deltaTime); //inputdir y+ is w 
    this->position = this->position +  (10*inputDir.x*this->right*deltaTime); //inputdir y+ is w 

    //simple fov change
    if(zoomIn){
        this->fov ++;

    }
    if(zoomOut){
        this->fov--;
    }
    
}



mat4 Camera::getViewMat()
{
    mat4 view = glm::lookAt(this->position,this->position + this->forward,this->up);
    return view;
}

mat4 Camera::getProjectionMat()
{
    mat4 proj = glm::perspective(glm::radians(this->fov),(float)this->width/(float)this->height,this->nearPlane,this->farPlane);
    return proj;
}