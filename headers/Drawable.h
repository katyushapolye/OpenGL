#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "Definitions.h"

#include "Shader.h"


class Drawable {
public:
    virtual ~Drawable() = default;
    virtual void draw(Shader* shader) = 0;

    ShaderType shaderType;

    
    // Method 4: Enum-based type identification
    enum class DrawableType {
        MODEL,
        MIRROR,
    };
    virtual DrawableType getType() = 0;
    virtual ShaderType getShaderType() = 0;
};

#endif
