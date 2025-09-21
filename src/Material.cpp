#include "../headers/Material.h"

Material::Material(Texture* diffuse, Texture* specular, Texture* normal, float shininess = 0.5f,std::string name) {
    this->diffuse = std::shared_ptr<Texture>(diffuse);
    this->specular = std::shared_ptr<Texture>(specular);
    this->normal = std::shared_ptr<Texture>(normal);
    this->shininess = shininess;
    this->name = name;
}