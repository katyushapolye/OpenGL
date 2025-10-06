#version 420 core
in vec2 fTexCoord;
in vec3 fVertexNormal;
in vec3 fFragPos;

out vec4 FragColor;

void main()
{


    FragColor = vec4(1.0,0.1,0.1,1.0);
    //float depth = gl_FragCoord.z; // divide by far for demonstration
    //FragColor = vec4(vec3(depth), 1.0);
}