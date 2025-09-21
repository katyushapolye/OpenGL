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
    std::shared_ptr<Texture> diffuse;
    std::shared_ptr<Texture> specular;
    std::shared_ptr<Texture> normal;
    
    float     shininess;
    Material(Texture* diffuse, Texture* specular, Texture* normal, float shininess,std::string name = "Material");

};
#endif


