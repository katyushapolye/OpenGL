#include "../headers/InstancedModel.h"

InstancedModel::InstancedModel(std::vector<unique_ptr<InstancedMesh>> meshes,std::vector<unique_ptr<Material>> materials){
    this->drawableType = DrawableType::INSTANCED_MODEL;
    this->renderGroup = RenderGroup::Opaque; //Instanced models do not support transparency
    
    this->meshes = std::move(meshes);
    this->materials = std::move(materials);
    this->shaderType = ShaderType::Instanced;

    this->instanceCount = 0;
    this->transforms.reserve(1000);
    //this->transforms.push_back(Transform()); //add at least one for good measure
    needUpdate = true;


}

void InstancedModel::addInstance(Transform transform){

    this->transforms.push_back(transform);
    instanceCount++;
    needUpdate = true;

}

void InstancedModel::killInstance(unsigned int index){
    if(index < this->transforms.size()){
    this->transforms.erase(this->transforms.begin() + index);
    }
    else{
        Log::write("[InstancedModel::setInstance] -  Index " + std::to_string(index) + " out of range in " +std::to_string(this->transforms.size())+ " in killInstance");
    }

}

void InstancedModel::setInstance(Transform transform, unsigned int index){
    if(index < this->transforms.size()){
        this->transforms.at(index) = transform;
        needUpdate = true;

    }
    else{
        Log::write("[InstancedModel::setInstance] -  Index " + std::to_string(index) + " out of range in " +std::to_string(this->transforms.size())+ " in setInstance");
    }
}

Transform InstancedModel::getInstance(unsigned int index){
    if(index < this->transforms.size()){
        return this->transforms.at(index);

    }
    else{
                Log::write("[InstancedModel::setInstance] -  Index " + std::to_string(index) + " out of range in " +std::to_string(this->transforms.size())+ " in getInstance");
                return Transform();
    }

}

RenderGroup InstancedModel::getRenderGroup(){
    return this->renderGroup;
}

DrawableType InstancedModel::getType(){
    return this->drawableType;
}

//updates the instance array, returns true if it was changed, false if it is still the same 
bool InstancedModel::updateInstanceArray(){
    if(needUpdate){
        this->instanceMatrices.clear();
        for(int i = 0;i<transforms.size();i++){
            this->instanceMatrices.push_back(transforms.at(i).getTransformMat());

        }
        return true;
    }
    else{
        return false;
    }
}

ShaderType InstancedModel::getShaderType(){
    return this->shaderType;

}


void InstancedModel::draw(Shader* shader){
 //create instance array, we will go for a needUpdate approach after debug
 bool wasUpdated = updateInstanceArray();


 for (const auto& mesh : this->meshes) {

        mesh->InstancedDraw(shader,this->materials[mesh->getMaterialIndex()].get(),this->instanceMatrices,wasUpdated);


    }

}