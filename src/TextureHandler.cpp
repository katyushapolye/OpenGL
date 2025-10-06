#include "../headers/TextureHandler.h"

std::map<std::string,shared_ptr<Texture>> TextureHandler::loadedTextures = std::map<std::string,shared_ptr<Texture>>();


shared_ptr<Texture> TextureHandler::loadTexture(std::string path,TextureType type){

    if(path == "DIFFUSE_FALLBACK"){
        Log::write("[TextureHandler::loadTexture] - Setting texture path to Diffuse fallback.png");
        path = "Resources/Textures/Fallback/fallback_white.png";
    }
    else if(path == "GENERIC_FALLBACK" || path == ""){
        Log::write("[TextureHandler::loadTexture] - Path was either empty or GENERIC_FALLBACK. Setting texture path to generic fallback.png");

        path = "Resources/Textures/Fallback/fallback.png";
    }



    if(loadedTextures.find(path) == loadedTextures.end() ){
        

        loadedTextures.insert (std::pair<std::string,shared_ptr<Texture>>(path,shared_ptr<Texture>(new Texture(path,type))));
        Log::write("[TextureHandler::loadTexture] - Loaded texture from path "+path + "- with transparency of value " + std::to_string(loadedTextures.at(path).get()->isTransparent()));
        return loadedTextures.at(path);

    }

    else{
        
        return loadedTextures.at(path);
    }

}

int TextureHandler::getLoadedTextureCount(){
    return loadedTextures.size();
}

