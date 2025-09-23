#ifndef SCENE_H
#define SCENE_H
#include "Definitions.h"
#include <map>
#include "Model.h"
#include "Light.h"

//This class stores all model and  lights informations
class Scene
{
private:
    std::vector<shared_ptr<Drawable>> models;
    std::map<LightType,std::vector<shared_ptr<Light>>> lights;


    
public:
    vec3 ambientLight;

    Scene();



    void addModel(shared_ptr<Drawable> model);
    
    void addLight(shared_ptr<DirectionalLight> light);
    void addLight(shared_ptr<PointLight> light);
    void addLight(shared_ptr<SpotLight> light);


    std::vector<shared_ptr<Drawable>> getModels();
    std::map<LightType,std::vector<shared_ptr<Light>>> getLights();


};




#endif