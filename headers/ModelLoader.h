#ifndef MODELLOADER_H
#define MODELLOADER_H
#include <memory>
#include "Model.h"
#include "TextureHandler.h"
#include <fstream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


class ModelLoader
{
private:
    static std::vector<Material*> loadMTLMaterial(std::string path);
    
public:

    static Model* loadFromObj(std::string path);

    static Model* loadFromObjWithAssimp(std::string path);


};

#endif


