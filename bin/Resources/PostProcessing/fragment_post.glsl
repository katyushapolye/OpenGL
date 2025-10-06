#version 440 core
out vec4 FragColor;
  


//screen buffers
uniform sampler2D screenTexture;
uniform sampler2D screenDepth;
uniform float time;

in vec2 TexCoords;
void main() {

    vec3 finalColor = texture(screenTexture, TexCoords).rgb;
   
    FragColor = vec4(finalColor, 1.0);
}