#include "../headers/Volumetric.h"

Volumetric::Volumetric(Transform t,float width,float height,float length){
    this->transform = t;
    this->width = width;
    this->height = height;
    this->length = length;

   glGenTextures(1, &gl_Density_Texture3D);
glBindTexture(GL_TEXTURE_3D, gl_Density_Texture3D);
glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// Choose ONE wrapping mode - CLAMP_TO_EDGE is safer
glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 1, 1, 1, 0, GL_RED, GL_FLOAT, nullptr);

    this->scatteringCoefficient = vec3(1.0,1.0,1.0);
    this->refractionIndex = 1.33;



}

void Volumetric::draw(Shader* shader){

}


//returns the axixs aligned distance to P
float Volumetric::distanceTo(vec3 p){
    vec3 cubemin = this->transform.getPosition() - vec3(width/2.0,height/2.0,length/2.0);
    vec3 cubemax = this->transform.getPosition() + vec3(width/2.0,height/2.0,length/2.0);
    
    // Check if point is inside the box
    bool inside = p.x >= cubemin.x && p.x <= cubemax.x &&
                  p.y >= cubemin.y && p.y <= cubemax.y &&
                  p.z >= cubemin.z && p.z <= cubemax.z;
    
    if (inside) {
        vec3 corners[8] = {
            cubemin,
            vec3(cubemax.x, cubemin.y, cubemin.z),
            vec3(cubemin.x, cubemax.y, cubemin.z),
            vec3(cubemin.x, cubemin.y, cubemax.z),
            vec3(cubemax.x, cubemax.y, cubemin.z),
            vec3(cubemax.x, cubemin.y, cubemax.z),
            vec3(cubemin.x, cubemax.y, cubemax.z),
            cubemax
        };
        
        float maxDist2 = 0.0f;
        for (int i = 0; i < 8; i++) {
            maxDist2 = glm::max(maxDist2, glm::length2(p - corners[i]));
        }
        return maxDist2;
    }
    
    vec3 clamped = vec3(glm::clamp(p.x,cubemin.x,cubemax.x),
                        glm::clamp(p.y,cubemin.y,cubemax.y),
                        glm::clamp(p.z,cubemin.z,cubemax.z));
    return glm::length(p-clamped);
}



float Volumetric::distance2To(vec3 p){
    vec3 cubemin = this->transform.getPosition() - vec3(width/2.0,height/2.0,length/2.0);
    vec3 cubemax = this->transform.getPosition() + vec3(width/2.0,height/2.0,length/2.0);
   
    // Check if point is inside the box
    bool inside = p.x >= cubemin.x && p.x <= cubemax.x &&
                  p.y >= cubemin.y && p.y <= cubemax.y &&
                  p.z >= cubemin.z && p.z <= cubemax.z;
   
    if (inside) {
        return std::numeric_limits<float>::max(); // Guaranteed to be "farthest"
    }
   
    vec3 clamped = vec3(glm::clamp(p.x,cubemin.x,cubemax.x),
                        glm::clamp(p.y,cubemin.y,cubemax.y),
                        glm::clamp(p.z,cubemin.z,cubemax.z));
    return glm::length2(p-clamped);
}



//foor now,  juts properly aling the Nx, Ny and Nz for the volumetric size
//we sample cell centers and we iterate over them as I,J,K (Y,X,Z);
void Volumetric::setDensityField(std::unique_ptr<float[]> field, unsigned int Nx, unsigned int Ny, unsigned int Nz){
    this->densityField = std::move(field);
    this->Nx = Nx; this->Ny = Ny; this->Nz = Nz;
    this->dh = width/Nx;
    this->size = Nx*Ny*Nz;

    glBindTexture(GL_TEXTURE_3D, this->gl_Density_Texture3D);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, Nx, Ny, Nz, 0, GL_RED, GL_FLOAT, this->densityField.get());
    glBindTexture(GL_TEXTURE_3D,0);


}

void Volumetric::bindDensityField(unsigned int unit){
        glActiveTexture(GL_TEXTURE0 + unit); 
        glBindTexture(GL_TEXTURE_3D, this->gl_Density_Texture3D);

}

DrawableType Volumetric::getType(){
    return DrawableType::VOLUMETRIC;
};
ShaderType Volumetric::getShaderType(){
    return ShaderType::Raymarch;
};
RenderGroup Volumetric::getRenderGroup(){
    return RenderGroup::Transparent;
};