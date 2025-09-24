#version 440 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vTangent;
layout (location = 3) in vec3 vBitangent;
layout (location = 4) in vec2 vTexCoords;
layout (location = 5) in vec2 vTexCoords2;  // lightmap UVs

uniform mat4 modelMat;
uniform mat3 normalMat;

out vec2 fTexCoord;
out vec3 fVertexNormal;
out vec3 fFragPos;
out mat3 fTBNMat; //Tangent bitangent Normal matrix space transformation matrix

//Matrix UBO
layout(std140, binding = 0) uniform Matrixes  {
    mat4 viewMat;
    mat4 projectionMat;







};
//Camera UBO (on frag shader)




void main()
{
    gl_Position = projectionMat*viewMat*modelMat* vec4(vPosition, 1.0);
    fFragPos = vec3(modelMat*vec4(vPosition, 1.0));
    fTexCoord = vTexCoords;
    fVertexNormal = normalMat * vNormal;

    vec3 T = normalize(vec3(modelMat * vec4(vTangent,   0.0)));
    vec3 N = normalize(vec3(modelMat * vec4(vNormal,    0.0)));

    T = normalize(T - dot(T, N) * N);//granshimtd process to retort the mat
    vec3 B = cross(N, T);

    fTBNMat = mat3(T, B, N);

    //
} 
