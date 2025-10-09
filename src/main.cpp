
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
#include "../headers/Utils.h"
using namespace std::chrono_literals;









int main()
{


    Log::initLog("gl_log.txt");
    Log::write("Program Initialized!");
    std::random_device rd;
    std::mt19937 gen(rd());

    



    Renderer renderer = Renderer(1024,768,"OpenGL Window"); //we need instantiate the renderer because it starts openGL

    Scene* scene = new Scene();
    scene->ambientLight = vec3(0.9,0.9,0.9);
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




    Model* m = ModelLoader::loadFromObj("Resources/Models/quad.obj");
    m->transform.rotateGlobal(vec3(-90,0,0));
    m->transform.setScale(vec3(10,10,10));
    m->transform.setPosition(vec3(0,0,10));
    scene->addModel(shared_ptr<Model>(m));

   scene->addModel(shared_ptr<Model>(   ModelLoader::loadFromObj("Resources/Models/gizmo.obj")));

    m = ModelLoader::loadFromObj("Resources/Models/cube.obj");
    m->transform.setPosition(vec3(-1,10,0));
    m->transform.setScale(vec3(0.2,0.2,0.2));
    m->transform.rotateGlobal(vec3(0,0,45));
    scene->addModel(shared_ptr<Model>(m));

    Transform t = Transform();
    t.setPosition(vec3(0,5,0));
    Volumetric* fluidView = new Volumetric(t,10,10,10);
    fluidView->scatteringCoefficient = vec3(0.2*2, 0.175*2, 0.1*2); // Slightly blue
    scene->addModel(shared_ptr<Volumetric>(fluidView));



    renderer.loadScene(scene);


    unsigned int Nx = 64, Ny = 64, Nz = 64;
    size_t totalSize = Nx * Ny * Nz;

    std::unique_ptr<float[]> densityField(new float[totalSize]());

    int currentFrame = 1;
    const int maxFrame = 40;

    while(renderer.isRunning()){
        // Read the current frame
        std::string filename = "Debug\\density_" + std::to_string(currentFrame) + ".txt";
        std::vector<float> vec = Utils::readGrid_DEBUG(filename.c_str(), Nx, Ny);

        // Fill the density field
        for (unsigned int k = 12; k < 54; ++k) {   // z
            for (unsigned int i = 0; i < Ny; ++i) { // y
                for (unsigned int j = 0; j < Nx; ++j) { // x
                    unsigned int index = j + Nx * (i + Ny * k);
                    densityField.get()[index] = (glm::clamp(0.0f,4.f,vec[i * Nx + j])/4.0); //we can test different

                    //functions later
                }
            }
        }

        // Update the fluid view
        fluidView->setDensityField(std::move(densityField), Nx, Ny, Nz);

        // Render the frame
        renderer.renderPass();

        // Move to next frame and loop back
        currentFrame++;
        if (currentFrame > maxFrame) {
            currentFrame = 1;
        }

        // Re-allocate for next iteration (since we moved densityField)
        densityField.reset(new float[totalSize]());
std::this_thread::sleep_for(33ms);
    }

    renderer.dispose();


    glfwTerminate();
    Log::closeLog();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly



