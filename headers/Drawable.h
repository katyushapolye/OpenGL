#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "Definitions.h"

#include "Shader.h"


class Drawable {
protected:

    ShaderType shaderType;
    RenderGroup renderGroup;
    DrawableType drawableType;
public:


    virtual ~Drawable() = default;
    virtual void draw(Shader* shader) = 0;

    
    // Method 4: Enum-based type identification


    virtual DrawableType getType() = 0;
    virtual ShaderType getShaderType() = 0;
    virtual RenderGroup getRenderGroup() = 0;
};

#endif
