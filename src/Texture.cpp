#include "../headers/Texture.h"

Texture::Texture(std::string path, TextureType type){
    stbi_set_flip_vertically_on_load(true); //to make the uv map easier
    unsigned char* rawTexture = stbi_load(path.c_str(),&(this->w),&(this->h),&(this->channels),4);
    if(rawTexture == nullptr){
        std::cout << "Failed to load texture!" << std::endl;
        return;
    }
    hasTransparency = false;
    for (int i = 0; i < w * h; i++) {
        unsigned char alpha = rawTexture[i * 4 + 3]; // 4th channel
        if (alpha < 255) {                     // anything not fully opaque
            hasTransparency = true;
            break;
        }
    }

    glGenTextures(1,&(this->gl_TexID));
    glBindTexture(GL_TEXTURE_2D,this->gl_TexID);

    //set texture options, we should be able to set these later
    //set texture anisotropy
    //GLfloat value, max_anisotropy = 8.0f;
    //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, & value);
    //value = (value > max_anisotropy) ? max_anisotropy : value;
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);
    //// Add this after your anisotropic filtering code
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.0f);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT); //repeatt at s
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT); //repeat at t
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //how to sample in case it downscales
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //how to sample in case of upscales


    //text target, mipmap level, store format, width,height, legacy stuff,format of source, source type, source
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,this->w,this->h,0,GL_RGBA,GL_UNSIGNED_BYTE,rawTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    this->type = type;

    stbi_image_free(rawTexture);



}

bool Texture::isTransparent(){
    return this->hasTransparency;
}

Texture::Texture(std::string path){
    stbi_set_flip_vertically_on_load(true); //to make the uv map easier
    unsigned char* rawTexture = stbi_load(path.c_str(),&(this->w),&(this->h),&(this->channels),4);
    //std::cout << path << std::endl;
    if(rawTexture == nullptr){
        Log::write("[Texture::Texture] - Failed to load texture at path:" + path);
        return;
    }

    
    hasTransparency = false;
    for (int i = 0; i < w * h; i++) {
        unsigned char alpha = rawTexture[i * 4 + 3]; // 4th channel
        if (alpha < 255) {                     // anything not fully opaque
            hasTransparency = true;
            break;
        }
    }
    
    glGenTextures(1,&(this->gl_TexID));
    glBindTexture(GL_TEXTURE_2D,this->gl_TexID);

    //set texture options, we should be able to set these later
    //set texture anisotropy
    //GLfloat value, max_anisotropy = 8.0f;
    //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, & value);
    //value = (value > max_anisotropy) ? max_anisotropy : value;
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);
    //// Add this after your anisotropic filtering code
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.0f);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP); //repeatt at s
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP); //repeat at t
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //how to sample in case it downscales
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //how to sample in case of upscales


    //text target, mipmap level, store format, width,height, legacy stuff,format of source, source type, source
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,this->w,this->h,0,GL_RGBA,GL_UNSIGNED_BYTE,rawTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    this->type = TextureType::DIFFUSE;

    glBindTexture(GL_TEXTURE_2D, 0); // Add this line to unbind

    stbi_image_free(rawTexture);



}
TextureType Texture::getType(){
    return this->type;
}

void Texture::bindTexture(unsigned int texUnity){
    glActiveTexture(GL_TEXTURE0+texUnity);
    glBindTexture(GL_TEXTURE_2D,this->gl_TexID);

}