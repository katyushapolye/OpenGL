#ifndef TEXTUREHANDLER_H
#define TEXTUREHANDLER_H
#include "Definitions.h"
#include "Texture.h"
#include <map>

//this static class loads and stores textures globably so we can avoid multiple loads of the same texture
class TextureHandler
{
private:
    static std::map<std::string,shared_ptr<Texture>> loadedTextures;
public:
    //retruns a shared ptr to a texture, if the texture was not loaded already, it loads it in gpu (via texture obj) and stores the texture ptr
    static shared_ptr<Texture> loadTexture(std::string path);

    static int getLoadedTextureCount();

};
#endif


