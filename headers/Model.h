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




public:
    bool hasOutline = false;
   
    
    Transform transform;
    Model(std::vector<unique_ptr<Mesh>> meshes,std::vector<unique_ptr<Material>> materials);





    //Drawable overridess
    DrawableType getType() override;
    ShaderType getShaderType() override;
    RenderGroup getRenderGroup() override;
    void draw(Shader* shader) override;





};
#endif

