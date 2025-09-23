#include "../headers/Scene.h"

Scene::Scene(){
    this->ambientLight = vec3(0.7,0.7,0.7);
    this->models = std::vector<shared_ptr<Drawable>>();
    models.reserve(16);

    lights[LightType::DIRECTIONAL].reserve(4);
    lights[LightType::POINT].reserve(16);
    lights[LightType::SPOT].reserve(16);


}

void Scene::addModel(shared_ptr<Drawable> model){
    this->models.push_back(model);
}

void Scene::addLight(shared_ptr<DirectionalLight> light){
    this->lights[light->getType()].push_back(light); //implicit cast to light
}
void Scene::addLight(shared_ptr<PointLight> light){
    this->lights[light->getType()].push_back(light);
}
void Scene::addLight(shared_ptr<SpotLight> light){
    this->lights[light->getType()].push_back(light);
}



std::vector<std::shared_ptr<Drawable>> Scene::getModels(){
    return this->models;
}


std::map<LightType,std::vector<shared_ptr<Light>>> Scene::getLights(){
    return this->lights;
}