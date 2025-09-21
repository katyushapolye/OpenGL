#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Log.h"
#include <memory>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::mat3;

struct Vertex {
    vec3 position;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 texCoords;
    vec2 texCoords2;
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



#endif