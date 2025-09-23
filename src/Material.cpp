#include "../headers/Material.h"

Material::Material(Texture* diffuse, Texture* specular, Texture* normal,Texture* reflection ,vec3 diffuseColor,float shininess,std::string name) {
    this->diffuseColor = diffuseColor;
    this->diffuseMap = shared_ptr<Texture>(diffuse);
    this->specularMap = shared_ptr<Texture>(specular);
    this->normalMap = shared_ptr<Texture>(normal);
    this->reflectionMap = shared_ptr<Texture>(reflection);

    this->shininess = shininess;
    this->name = name;
}