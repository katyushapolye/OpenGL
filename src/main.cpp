#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <random>
#include <filesystem>

#include "../headers/Definitions.h"
#include "../headers/Mesh.h"
#include"../headers/Shader.h"
#include "../headers/Texture.h"
#include "../headers/Transform.h"
#include "../headers/Camera.h"
#include "../headers/Renderer.h"
#include "../headers/ModelLoader.h"
#include "../headers/Log.h"
#include "../headers/Utils.h"
#include "../headers/UI.h"




using namespace std::chrono_literals;

int main(int argc, char *argv[])
{
    #ifndef _WIN32
    std::filesystem::current_path(
        std::filesystem::path(argv[0]).parent_path()
    );
    #endif
    

    Log::initLog("gl_log.txt");
    Log::write("CWD: " + std::filesystem::current_path().string());
    Log::write("argv[0]: " + std::string(argv[0]));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);



    Renderer renderer = Renderer(1024,768,"OpenGL Window");



    Scene* scene = new Scene();
    scene->ambientLight = vec3(0.9,0.9,0.9);

    DirectionalLight* l= new DirectionalLight(vec3(0.0f, 0.0f,0.0f),vec3(1.0,1.0,1.0),1.0f);
    l->transform.setPosition(vec3(0,7, 5));
    l->transform.lookAt(vec3(0,0,0));
    scene->addLight(shared_ptr<DirectionalLight>(l));

    Model* m = ModelLoader::loadFromObj("Resources/Models/quad.obj");
    m->transform.rotateGlobal(vec3(0,0,0));
    m->transform.setScale(vec3(1,1,1));
    m->transform.setPosition(vec3(0,0,0));
    scene->addModel(shared_ptr<Model>(m));

    m = ModelLoader::loadFromObj("Resources/Models/windmill.obj");
    m->transform.rotateGlobal(vec3(0,0,0));
    m->transform.setScale(vec3(5,5,5));
    m->transform.setPosition(vec3(0,0,0));
    scene->addModel(shared_ptr<Model>(m));




    InstancedModel* inst = ModelLoader::loadFromObjAsInstanced("Resources/Models/grassblade.obj");

    float size = 20.0f;
    float dh = 0.05f;

    int nx = static_cast<int>(size / dh);

    std::cout << nx*nx << std::endl;;
    for (int i = 0; i <= nx; ++i)
    {
        float x = -size * 0.5f + i * dh;

        for (int j = 0; j <= nx; ++j)
        {
            float z = -size * 0.5f + j * dh;

            float randomValue = dist(gen) - 0.5;
            float jitter = dh * 0.4f * randomValue;

            float dx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * jitter;
            float dz = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * jitter;

            Transform t;
            t.setRotation(vec3(0,90+20*randomValue,0));
            t.setScale(0.3f*vec3(randomValue+0.8f, randomValue+0.8f, randomValue+0.8f));
            t.setPosition(vec3(x + dx, 0.0f, z + dz));

            inst->addInstance(t);
        }
    }
    scene->addModel(shared_ptr<InstancedModel>(inst));



    renderer.loadScene(scene);
    renderer.addUIElement(std::unique_ptr<Text>(new Text("Hello World!",vec2(0.0,0.0))));







    while(renderer.isRunning()){




        renderer.renderPass();


        
    }


    renderer.dispose();

    Log::closeLog();
    return 0;
}