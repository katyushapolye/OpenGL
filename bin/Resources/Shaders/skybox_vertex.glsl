#version 420 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projectionMat;
uniform mat4 viewMat; //we have removed the translation from this in the c++ code, so the cube is always inside the camera, since it also has no model mat

void main()
{
    TexCoords = aPos;
    vec4 pos = projectionMat * viewMat * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  