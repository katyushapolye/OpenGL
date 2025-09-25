
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <glm/glm.hpp>

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
    



    Renderer renderer = Renderer(800,600,"OpenGL Window"); //we need instantiate the renderer because it starts openGL

    Scene* scene = new Scene();
    scene->ambientLight = vec3(0.1,0.1,0.1);
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

    m = ModelLoader::loadFromObj("Models/grassleaves.obj");
    m->transform.setPosition(vec3(0.0,-1,2.0));
    m->transform.rotateLocal(vec3(0,180,0));
    scene->addModel(shared_ptr<Model>(m));

    m = ModelLoader::loadFromObj("Models/teapot.obj");
    m->transform.setPosition(vec3(1.75,1,2.5));
    m->transform.setScale(vec3(5,5,5));
    scene->addModel(shared_ptr<Model>(m));


    //DirectionalLight* l= new DirectionalLight(vec3(0.0f, 0.0f,0.0f),vec3(1.0,1.0,1.0),1.0f);
    //l->transform.setRotation(vec3(60.0,0.0,0.0f));
    //scene->addLight(shared_ptr<DirectionalLight>(l));

    PointLight* l1 = new PointLight(vec3(0.0,3.0,2.5),vec3(1.0,1.0,1.0),5.0);
    scene->addLight(shared_ptr<PointLight>(l1));


    InstancedModel* iM = ModelLoader::loadFromObjAsInstanced("Models/grassblade.obj");
    //create lots of instances on thety model
    for(int i = 0;i<100;i++){
        for(int j = 0;j<100;j++){
            Transform t;
            t.setPosition(vec3(i*0.25,0,j*0.25));
            iM->transforms.push_back(t);
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



