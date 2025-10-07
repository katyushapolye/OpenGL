#ifndef VOLUMETRIC_H
#define VOLUMETRIC_H

#include "Definitions.h"
#include "Transform.h"
#include "Drawable.h"


class Volumetric : public Drawable
{
private:

    float* densityField; //linear representation of 3d density field
    unsigned int Nx,Ny,Nz;
    unsigned int size;
    float dh; //we use a equally spaced grid

    //if you dont want to use a discrete grid, we will also provide some random noise fucntions, but this is for now mainly for fluid sim

public:
    float width, length,height;
    Transform transform;
    Volumetric(Transform t,float width,float height,float length);


    //returns the distance to the point from the border of the cube
    
    float distanceTo(vec3 p);
    float distance2To(vec3 p);
    
    DrawableType getType() override;
    ShaderType getShaderType() override;
    RenderGroup getRenderGroup() override;

    void draw(Shader* shader) override;


};




#endif