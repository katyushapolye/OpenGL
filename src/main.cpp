
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
    scene->addLight(shared_ptr<Light>(new Light(LightType::POINT, vec3(1.0f, 1.0f,1.0f),5.0f)));

    renderer.loadScene(scene);
    renderer.loop();
    renderer.dispose();


    glfwTerminate();
    Log::closeLog();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly



