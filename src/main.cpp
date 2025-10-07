
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

    



    Renderer renderer = Renderer(1280,720,"OpenGL Window"); //we need instantiate the renderer because it starts openGL

    Scene* scene = new Scene();
    scene->ambientLight = vec3(0.3,0.3,0.3);
    /*Grass scene
    //scene->addModel(shared_ptr<Model>(   ModelLoader::loadFromObj("Models/gizmo.obj")));


    DirectionalLight* l= new DirectionalLight(vec3(0.0f, 0.0f,0.0f),vec3(1.0,1.0,1.0),0.7f);
    l->transform.setRotation(vec3(80,0.0,0.0f));
    scene->addLight(shared_ptr<DirectionalLight>(l));


    InstancedModel* iM = ModelLoader::loadFromObjAsInstanced("Models/grassblade.obj");
    //create lots of instances on thety model
    std::uniform_real_distribution<float> dis(-0.2f, 0.2f);
    std::uniform_real_distribution<float> dis2(-0.4f, 0.4f);
    std::uniform_real_distribution<float> rot(-15.0, 15.0);
    float offset = 0.25;
    for(int i = 0; i < 500; i++){
        for(int j = 0; j < 500; j++){
            Transform t;

            // Generate random offsets for x and z components
            float randX = dis(gen);
            float randZ = dis(gen);
            float randY = dis2(gen);
            float randRot = rot(gen);

            float xPos = i * offset + randX - 62.5;
            float zPos =  j * offset  + randZ - 62.5;

            if(xPos < 2.5 && xPos > -2.5 && zPos <2.5 && zPos > -2.5){
                continue;
            }
                t.setPosition(glm::vec3(xPos, -1.0f + randY, zPos));
                t.setRotation(glm::vec3(0.0f, randRot, 0.0f));
                iM->addInstance(t);
        }
    }
    scene->addModel(shared_ptr<InstancedModel>(iM));
    
    //scene->addModel(shared_ptr<Model>( ModelLoader::loadFromObj("Models/surface.obj")));
   // scene->addModel(shared_ptr<Model>(   ModelLoader::loadFromObj("Models/gizmo.obj")));

    Model* m = ModelLoader::loadFromObj("Models/terrain.obj");
    m->transform.setPosition(vec3(0,-1,0));
    m->transform.setScale(vec3(10,10,10));
    scene->addModel(shared_ptr<Model>(m));

    m = ModelLoader::loadFromObj("Models/well.obj");
    m->transform.setPosition(vec3(0,-1,0));
    m->transform.setScale(vec3(1,1,1));
    scene->addModel(shared_ptr<Model>(m));

    /*
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
    
    


    DirectionalLight* l= new DirectionalLight(vec3(0.0f, 0.0f,0.0f),vec3(1.0,1.0,1.0),0.7f);
    l->transform.setRotation(vec3(70,0.0,0.0f));
    scene->addLight(shared_ptr<DirectionalLight>(l));
        PointLight* l1 = new PointLight(vec3(0.0,5.0,5.0),vec3(1.0,1.0,1.0),1.0);
    scene->addLight(shared_ptr<PointLight>(l1));
    
    /*
    SpotLight* l= new SpotLight(vec3(0,3,0),vec3(0,0,0),vec3(1.0,1.0,1.0),10.0f,12.5f,17.5f,10.0f);
    l->transform.setRotation(vec3(90.0,0.0,0.0f));
    //scene->addLight(shared_ptr<SpotLight>(l));*/
    //SpotLight* l= new SpotLight(vec3(0,0,5),vec3(0,0,0),vec3(1.0,1.0,1.0),15.0f,12.5f,17.5f,10.0f);
    //l->transform.lookAt(vec3(0,0,0));
    //scene->addLight(shared_ptr<SpotLight>(l));

    //SpotLight* ls= new SpotLight(vec3(0,5,5),vec3(0,0,0),vec3(1.0,1.0,1.0),25.0f,15.0f,60.0f,10.0f);
    //ls->transform.lookAt(vec3(0,0,0));
    //scene->addLight(shared_ptr<SpotLight>(ls));

    DirectionalLight* l= new DirectionalLight(vec3(0.0f, 0.0f,0.0f),vec3(1.0,1.0,1.0),1.0f);
    l->transform.setPosition(vec3(0,7, 5));
    l->transform.lookAt(vec3(0,0,0));
    scene->addLight(shared_ptr<DirectionalLight>(l));




    //Model* m = ModelLoader::loadFromObj("Resources/Models/cube.obj");
    //m->transform.setScale(vec3(0.5,0.5,0.5));
    //m->transform.setPosition(vec3(0,1,7));
    //scene->addModel(shared_ptr<Model>(m));

   scene->addModel(shared_ptr<Model>(   ModelLoader::loadFromObj("Resources/Models/gizmo.obj")));


    //Model* m = ModelLoader::loadFromObj("Resources/Models/window.obj");
    //m->transform.setScale(vec3(5.0,5.0,5.0));
    //m->transform.setPosition(vec3(0,-5,5.0));
    //scene->addModel(shared_ptr<Model>(m));


    scene->addModel(shared_ptr<Volumetric>(new Volumetric(Transform(),10,10,10)));

    renderer.loadScene(scene);
    renderer.loop();
    renderer.dispose();


    glfwTerminate();
    Log::closeLog();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly



