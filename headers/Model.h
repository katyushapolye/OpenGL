#ifndef MODEL_H
#define MODEL_H
#include "Definitions.h"
#include "Mesh.h"
#include "Material.h"
#include "Transform.h"
#include <memory>
#include <string>   
#include <regex>    


class Model
{
private:
    std::vector<std::unique_ptr<Mesh>> meshes;
    std::vector<std::unique_ptr<Material>> materials; // Also fixed the type here
   
public:
    Transform transform;
   

    Model(std::vector<std::unique_ptr<Mesh>> meshes,std::vector<std::unique_ptr<Material>> materials);

    void draw(const std::unique_ptr<Shader>& shader);
};
#endif

