
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <random>

#include "../headers/Definitions.h"
#include "../headers/Mesh.h"
#include"../headers/Shader.h"
#include "../headers/Texture.h"
#include "../headers/Transform.h"
#include "../headers/Camera.h"
#include "../headers/Renderer.h"
#include "../headers/ModelLoader.h"
#include "../headers/Log.h"










int main()
{


    Log::initLog("gl_log.txt");
    Log::write("Program Initialized!");
    std::random_device rd;
    std::mt19937 gen(rd());

    



    Renderer renderer = Renderer(800,600,"OpenGL Window"); //we need instantiate the renderer because it starts openGL

    Scene* scene = new Scene();
    scene->ambientLight = vec3(0.7,0.7,0.7);
        scene->addModel(shared_ptr<Model>(   ModelLoader::loadFromObj("Models/gizmo.obj")));
    
    scene->addModel(shared_ptr<Model>( ModelLoader::loadFromObj("Models/surface.obj")));
    scene->addModel(shared_ptr<Model>(   ModelLoader::loadFromObj("Models/gizmo.obj")));
    Model* m = ModelLoader::loadFromObj("Models/sphere.obj");
    m->hasOutline = true;
    m->transform.setPosition(vec3(2.5,0,0));
    scene->addModel(shared_ptr<Model>(m));

    m = ModelLoader::loadFromObj("Models/window.obj");
    m->transform.setPosition(vec3(0,-1,0));
    m->transform.rotateLocal(vec3(0,180,0));
    scene->addModel(shared_ptr<Model>(m));

    m = ModelLoader::loadFromObj("Models/window.obj");
    m->transform.setPosition(vec3(0,-1,0.5));
    m->transform.rotateLocal(vec3(0,180,0));
    scene->addModel(shared_ptr<Model>(m));

    m = ModelLoader::loadFromObj("Models/window.obj");
    m->transform.setPosition(vec3(0,-1,1.0));
    m->transform.rotateLocal(vec3(0,180,0));
    scene->addModel(shared_ptr<Model>(m));


    m = ModelLoader::loadFromObj("Models/teapot.obj");
    m->transform.setPosition(vec3(1.75,1,2.5));
    m->transform.setScale(vec3(5,5,5));
    scene->addModel(shared_ptr<Model>(m));
    


    DirectionalLight* l= new DirectionalLight(vec3(0.0f, 0.0f,0.0f),vec3(1.0,1.0,1.0),1.0f);
    l->transform.setRotation(vec3(45,0.0,0.0f));
    scene->addLight(shared_ptr<DirectionalLight>(l));
    /*
    SpotLight* l= new SpotLight(vec3(0,3,0),vec3(0,0,0),vec3(1.0,1.0,1.0),10.0f,12.5f,17.5f,10.0f);
    l->transform.setRotation(vec3(90.0,0.0,0.0f));
    scene->addLight(shared_ptr<SpotLight>(l));*/

    //PointLight* l1 = new PointLight(vec3(0.0,3.0,2.5),vec3(1.0,1.0,1.0),5.0);
    //scene->addLight(shared_ptr<PointLight>(l1));


    InstancedModel* iM = ModelLoader::loadFromObjAsInstanced("Models/grassblade.obj");
    //create lots of instances on thety model
    std::uniform_real_distribution<float> dis(-0.2f, 0.2f);
    std::uniform_real_distribution<float> rot(-15.0, 15.0);
    float offset = 0.25;
    for(int i = 0; i < 100; i++){
        for(int j = 0; j < 100; j++){
            Transform t;

            // Generate random offsets for x and z components
            float randX = dis(gen);
            float randZ = dis(gen);
            float randRot = rot(gen);


            t.setPosition(glm::vec3(i * offset + randX - 5.0, -1.0f, j * offset  + randZ - 5.0));
            t.setRotation(glm::vec3(0.0f, randRot, 0.0f));
            iM->addInstance(t);
        }
    }
    scene->addModel(shared_ptr<InstancedModel>(iM));





    renderer.loadScene(scene);
    renderer.loop();
    renderer.dispose();


    glfwTerminate();
    Log::closeLog();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly



