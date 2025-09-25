#include "../headers/InstancedModel.h"

InstancedModel::InstancedModel(std::vector<unique_ptr<InstancedMesh>> meshes,std::vector<unique_ptr<Material>> materials){
    this->drawableType = DrawableType::INSTANCED_MODEL;
    this->renderGroup = RenderGroup::Opaque; //Instanced models do not support transparency
    
    this->meshes = std::move(meshes);
    this->materials = std::move(materials);
    this->shaderType = ShaderType::Instanced;


}

RenderGroup InstancedModel::getRenderGroup(){
    return this->renderGroup;
}

DrawableType InstancedModel::getType(){
    return this->drawableType;
}

void InstancedModel::updateInstanceArray(){
    this->instanceMatrices.clear();
    for(int i = 0;i<transforms.size();i++){
        this->instanceMatrices.push_back(transforms.at(i).getTransformMat());

    }
}

ShaderType InstancedModel::getShaderType(){
    return this->shaderType;

}


void InstancedModel::draw(Shader* shader){
 //create instance array, we will go for a needUpdate approach after debug
 updateInstanceArray();


 for (const auto& mesh : this->meshes) {

        mesh->InstancedDraw(shader,this->materials[mesh->getMaterialIndex()].get(),this->instanceMatrices);


    }

}