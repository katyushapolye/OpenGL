#include "../headers/TextureHandler.h"

std::map<std::string,shared_ptr<Texture>> TextureHandler::loadedTextures = std::map<std::string,shared_ptr<Texture>>();


shared_ptr<Texture> TextureHandler::loadTexture(std::string path){

    if(path == ""){
        Log::write("[TextureHandler::loadTexture] - Loadtexture path was empty, defaulting to fallback texture");

        path = "Textures/backup/no_spec.png";
    }



    if(loadedTextures.find(path) == loadedTextures.end() ){
        

        loadedTextures.insert (std::pair<std::string,shared_ptr<Texture>>(path,shared_ptr<Texture>(new Texture(path))));
        return loadedTextures.at(path);

    }

    else{
        
        return loadedTextures.at(path);
    }

}

int TextureHandler::getLoadedTextureCount(){
    return loadedTextures.size();
}

