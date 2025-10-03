#version 440 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

const float MAGNITUDE = 0.4;

layout(std140, binding = 0) uniform Matrixes {
    mat4 viewMat;
    mat4 projectionMat;
};

void GenerateLine(int index)
{
    gl_Position = projectionMat * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = projectionMat * (gl_in[index].gl_Position +
                                vec4(gs_in[index].normal, 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0);
    GenerateLine(1);
    GenerateLine(2);
}