#version 440 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

layout (location = 3) in mat4 instanceMatrix; //(loc3 4 5 and 6 will be the mat4 s)



uniform mat3 normalMat;

out vec2 fTexCoord;
out vec3 fVertexNormal;
out vec3 fFragPos;


//Matrix UBO
layout(std140, binding = 0) uniform Matrixes  {
    mat4 viewMat;
    mat4 projectionMat;

};
//Camera UBO (on frag shader)




void main()
{
    gl_Position = projectionMat*viewMat*instanceMatrix* vec4(vPosition, 1.0);
    fFragPos = vec3(instanceMatrix*vec4(vPosition, 1.0));
    fTexCoord = vTexCoords;
    fVertexNormal = normalMat * vNormal;

} 
