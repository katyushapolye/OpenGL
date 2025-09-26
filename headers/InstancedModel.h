#ifndef INSTANCEDMODEL_H
#define INSTANCEDMODEL_H
#include "Definitions.h"
#include "InstancedMesh.h"
#include "Material.h"
#include "Transform.h"
#include "Drawable.h"
#include <memory>
#include <string>   
#include <regex>    

//Similar to Model, but it creates a draw call using the instanced tech, only use if you're planning on drawing 1000+ of the same model. It also uses
//instanced mesh class, since it has different attributes
class InstancedModel : public Drawable
{
private:
    std::vector<unique_ptr<InstancedMesh>> meshes;
    std::vector<unique_ptr<Material>> materials; // Also fixed the type here

    std::vector<glm::mat4> instanceMatrices;  // CPU-side storage
                                              // GPU buffer that constais all the instanceMatrixes
                                                         // The idea is to send all instanceMat (the model mat for each instance) in one go to render it

    bool updateInstanceArray();   

    std::vector<Transform> transforms;    
    unsigned int instanceCount;
    bool needUpdate;

public:



    //Instanced models do not support normal maps, nor transparency(since they are too many, sorting them would be a nightmare)
    InstancedModel(std::vector<unique_ptr<InstancedMesh>> meshes,std::vector<unique_ptr<Material>> materials);





    //Drawable overridess
    DrawableType getType() override;
    ShaderType getShaderType() override;
    RenderGroup getRenderGroup() override;
    void draw(Shader* shade) override;

    void setInstance(Transform transform, unsigned int index);
    void addInstance(Transform transform);
    void killInstance(unsigned int index);
    Transform getInstance(unsigned int index);
    





};
#endif