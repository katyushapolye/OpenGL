#include "../headers/Model.h"






Model::Model(std::vector<std::unique_ptr<Mesh>> meshes,std::vector<std::unique_ptr<Material>> material){


        this->meshes = std::move(meshes);
        this->materials = std::move(material);
        //materials.push_back(std::unique_ptr<Material> (new Material(testTexture, testSpecularTexture, testTexture, 0.9f)));


}



void Model::draw(const std::unique_ptr<Shader>& shader) {



    for (const auto& mesh : this->meshes) {
        mesh->draw(shader,this->materials[mesh->getMaterialIndex()]);
        //std::cout  << mesh->getMaterialIndex() << ":"<<this->materials[mesh->getMaterialIndex()].get()->name << std::endl;

    }
}