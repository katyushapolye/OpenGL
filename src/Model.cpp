#include "../headers/Model.h"






Model::Model(std::vector<std::unique_ptr<Mesh>> meshes,std::vector<std::unique_ptr<Material>> material){


        this->meshes = std::move(meshes);
        this->materials = std::move(material);
        this->shaderType = ShaderType::Lit;
        //check our type by inividdualyy checking all material textures

        for(auto mat = materials.begin();mat != materials.end();mat++ ){
            if(mat->get()->diffuse->isTransparent()){
                this->renderGroup = RenderGroup::Transparent;
            }
            else{
                this->renderGroup = RenderGroup::Opaque;
            }
        }
}



void Model::draw(const std::unique_ptr<Shader>& shader) {



    for (const auto& mesh : this->meshes) {
        mesh->draw(shader,this->materials[mesh->getMaterialIndex()]);
        //std::cout  << mesh->getMaterialIndex() << ":"<<this->materials[mesh->getMaterialIndex()].get()->name << std::endl;

    }
}

ShaderType Model::getShaderType(){
    return this->shaderType;

}

Drawable::DrawableType Model::getType(){
    return Drawable::DrawableType::MODEL;
}

RenderGroup Model::getRenderGroup(){
    return this->renderGroup;
}