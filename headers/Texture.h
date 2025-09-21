#ifndef TEXTURE_H
#define TEXTURE_H


#include <iostream>
#include <vector>

#include "Definitions.h"
#include "stb_image.h"

class Texture{

private:
    unsigned int gl_TexID;

    int w,h,channels;

    TextureType type;



public:
    Texture(std::string path);
    Texture(std::string path, TextureType type);


    TextureType getType();
    void bindTexture(unsigned int texUnity);



};

#endif