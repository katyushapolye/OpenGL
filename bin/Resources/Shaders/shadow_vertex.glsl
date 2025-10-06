#version 440 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMat;
uniform mat4 modelMat;

void main()
{
    gl_Position = lightSpaceMat * modelMat * vec4(aPos, 1.0);
}  