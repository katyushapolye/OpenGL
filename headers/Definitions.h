#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include "Log.h"
#include <memory>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::mat3;

using std::shared_ptr;
using std::unique_ptr;

struct Vertex {
    vec3 position;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 texCoords;
    vec2 texCoords2;
};

enum class DrawableType {
        MODEL,
        MIRROR,
};

enum class TextureType {
    DIFFUSE,
    SPECULAR,
    NORMAL
};

enum class LightType {
    DIRECTIONAL,
    POINT,
    SPOT
};

enum class ShaderType{ //should be put in render order (Lit first, Unlit second...)
    Lit,
    Outline,
};

enum class RenderGroup{
    Opaque,
    Transparent
};

const unsigned int SHADER_COUNT = 2;



#endif