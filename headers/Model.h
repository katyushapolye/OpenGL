#ifndef MODEL_H
#define MODEL_H
#include "Definitions.h"
#include "Mesh.h"
#include "Material.h"
#include "Transform.h"
#include "Drawable.h"
#include <memory>
#include <string>   
#include <regex>    


class Model : public Drawable
{
private:
    std::vector<unique_ptr<Mesh>> meshes;
    std::vector<unique_ptr<Material>> materials; // Also fixed the type here

    RenderGroup renderGroup;



public:
    bool hasOutline = false;
   
    
    Transform transform;
    Model(std::vector<unique_ptr<Mesh>> meshes,std::vector<unique_ptr<Material>> materials);
    RenderGroup getRenderGroup();




    //Drawable overridess
    ShaderType getShaderType() override;
    void draw(Shader* shader) override;
    DrawableType getType() override;




};
#endif

