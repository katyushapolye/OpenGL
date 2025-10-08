#ifndef VOLUMETRIC_H
#define VOLUMETRIC_H

#include "Definitions.h"
#include "Transform.h"
#include "Drawable.h"


class Volumetric : public Drawable
{
private:

    std::unique_ptr<float[]> densityField; //linear representation of 3d density field
    unsigned int Nx,Ny,Nz;               //we will ship to the gpu as a 3d texture
    unsigned int size;
    float dh; //we use a equally spaced grid

    unsigned int gl_Density_Texture3D;

    //if you dont want to use a discrete grid, we will also provide some random noise fucntions, but this is for now mainly for fluid sim

public:
    float width, length,height;

    vec3 scatteringCoefficient;
    float refractionIndex;

    Transform transform;
    Volumetric(Transform t,float width,float height,float length);


    //returns the distance to the point from the border of the cube
    
    float distanceTo(vec3 p);
    float distance2To(vec3 p);
    
    DrawableType getType() override;
    ShaderType getShaderType() override;
    RenderGroup getRenderGroup() override;

    void setDensityField(std::unique_ptr<float[]> field, unsigned int Nx,unsigned int Ny, unsigned int Nz);

    void draw(Shader* shader) override;

    void bindDensityField(unsigned int unit);


};




#endif