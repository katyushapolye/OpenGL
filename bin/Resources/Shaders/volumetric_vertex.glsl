//pos processing shader. It receives a quad that covers the entire camera view. 
#version 440 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

out mat4 inverseViewMat;//we will use this to calculate the direction vector of our frag for ray marching
out mat4 inverseProjectionMat; //we will put these in the UBO
uniform mat4 modelMat;

layout(std140, binding = 0) uniform Matrixes  {
    mat4 viewMat;
    mat4 projectionMat;

};

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
    TexCoords = aTexCoords;
    inverseProjectionMat = inverse(projectionMat);
    inverseViewMat = inverse(viewMat);
}