
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
    scene->addModel(shared_ptr<Model>( ModelLoader::loadFromObjWithAssimp("Models/surface.obj")));
    scene->addModel(shared_ptr<Model>(   ModelLoader::loadFromObjWithAssimp("Models/gizmo.obj")));
    Model* m = ModelLoader::loadFromObjWithAssimp("Models/sphere.obj");
    m->hasOutline = true;
    m->transform.setPosition(vec3(2.5,0,0));
    scene->addModel(shared_ptr<Model>(m));

    m = ModelLoader::loadFromObjWithAssimp("Models/window.obj");
    m->transform.setPosition(vec3(0,-1,0));
    m->transform.rotateLocal(vec3(0,180,0));
    scene->addModel(shared_ptr<Model>(m));

    m = ModelLoader::loadFromObjWithAssimp("Models/window.obj");
    m->transform.setPosition(vec3(0,-1,0.5));
    m->transform.rotateLocal(vec3(0,180,0));
    scene->addModel(shared_ptr<Model>(m));

    m = ModelLoader::loadFromObjWithAssimp("Models/window.obj");
    m->transform.setPosition(vec3(0,-1,1.0));
    m->transform.rotateLocal(vec3(0,180,0));
    scene->addModel(shared_ptr<Model>(m));

    m = ModelLoader::loadFromObjWithAssimp("Models/grassleaves.obj");
    m->transform.setPosition(vec3(0.0,-1,2.0));
    m->transform.rotateLocal(vec3(0,180,0));
    scene->addModel(shared_ptr<Model>(m));


    



    Light* l= new Light(LightType::POINT, vec3(1.0f, 1.0f,1.0f),10.0f);
    l->transform.setPosition(vec3(0,2.0f,2.0f));
    scene->addLight(shared_ptr<Light>(l));




    renderer.loadScene(scene);
    renderer.loop();
    renderer.dispose();


    glfwTerminate();
    Log::closeLog();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly



