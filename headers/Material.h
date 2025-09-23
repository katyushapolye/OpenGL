#ifndef MATERIAL_H
#define MATERIAL_H
#include "Texture.h"
#include <memory>

//we should use smart pointers here
class Material
{
private:

    
public:
    std::string name;
    vec3 diffuseColor; //we  will use these in case we havent set a texturee

    
    

    shared_ptr<Texture> diffuseMap;
    shared_ptr<Texture> specularMap;
    shared_ptr<Texture> reflectionMap;
    shared_ptr<Texture> normalMap;
    float     shininess;

    Material(Texture* diffuse, Texture* specular, Texture* normal, Texture* reflection,vec3 diffuseColor = vec3(1.0,1.0,1.0),float shininess = 0.25f,std::string name = "Material");

};
#endif


