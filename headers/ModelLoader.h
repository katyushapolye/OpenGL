#ifndef MODELLOADER_H
#define MODELLOADER_H
#include <memory>
#include "Model.h"
#include "InstancedModel.h"
#include "TextureHandler.h"
#include <fstream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


class ModelLoader
{
private:

    
public:

    static Model* loadFromObj(std::string path);
    static InstancedModel* loadFromObjAsInstanced(std::string path);


};

#endif


