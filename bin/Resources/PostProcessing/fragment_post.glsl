#version 440 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform sampler2D screenDepth;
uniform float time;

// Set to 0 for normal FXAA, 1 for offset visualization, 2 for simple blur test


// Convert RGB to luma
float rgb2luma(vec3 rgb) {
    return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}

void main() {
    vec3 finalColor = texture(screenTexture,TexCoords).rgb;
    FragColor = vec4(finalColor, 1.0);
  
}