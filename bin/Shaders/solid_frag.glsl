in vec2 fTexCoord;
in vec3 fVertexNormal;
in vec3 fFragPos;

out vec4 FragColor;

void main()
{


    FragColor = vec4(0.1,0.1,0.1);
    //float depth = gl_FragCoord.z; // divide by far for demonstration
    //FragColor = vec4(vec3(depth), 1.0);
}