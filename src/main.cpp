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
#include "../headers/UI.h"



using namespace std::chrono_literals;

int main()
{
    Log::initLog("gl_log.txt");
    Log::write("Program Initialized!");
    std::random_device rd;
    std::mt19937 gen(rd());

    Renderer renderer = Renderer(1024,768,"OpenGL Window");



    Scene* scene = new Scene();
    scene->ambientLight = vec3(0.9,0.9,0.9);

    DirectionalLight* l= new DirectionalLight(vec3(0.0f, 0.0f,0.0f),vec3(1.0,1.0,1.0),1.0f);
    l->transform.setPosition(vec3(0,7, 5));
    l->transform.lookAt(vec3(0,0,0));
    scene->addLight(shared_ptr<DirectionalLight>(l));

    Model* m = ModelLoader::loadFromObj("Resources/Models/quad.obj");
    m->transform.rotateGlobal(vec3(-90,0,0));
    m->transform.setScale(vec3(10,10,10));
    m->transform.setPosition(vec3(0,0,10));
    scene->addModel(shared_ptr<Model>(m));

   //scene->addModel(shared_ptr<Model>(ModelLoader::loadFromObj("Resources/Models/gizmo.obj")));

    //m = ModelLoader::loadFromObj("Resources/Models/cube.obj");
    //m->transform.setPosition(vec3(-1,0,0));
    //m->transform.setScale(vec3(0.2,10,0.2));
    //m->transform.rotateGlobal(vec3(0,0,45));
    //scene->addModel(shared_ptr<Model>(m));

    Transform t = Transform();



    
    Volumetric* fluidView = new Volumetric(t,5,5,5);
    fluidView->transform.setPosition(vec3(1.25,1.25,0));
    fluidView->transform.rotateGlobal(vec3(0,0,90));

    fluidView->densityMultiplier = 0.02f;
    fluidView->scatteringCoefficient = vec3(0.003, 0.001, 0.0);
    scene->addModel(shared_ptr<Volumetric>(fluidView));

    renderer.loadScene(scene);

    int Nx = 256, Ny = 256, Nz = 256;
    float dh = 1.0/256.0;
    size_t totalSize = Nx * Ny * Nz;








    renderer.addUIElement(std::move(unique_ptr<UIElement>(new Text("Camera Transform: ",vec2(0.0f,0.0f)))));


    renderer.addUIElement(std::move(unique_ptr<UIElement>(new Slider("DensityMultiplier",vec2(0.0f,0.0f), &fluidView->densityMultiplier, 0.0f, 1.0f,true))));
    renderer.addUIElement(std::move(unique_ptr<UIElement>(new Slider("Scattering R",vec2(0.0f,0.0f), &fluidView->scatteringCoefficient.x, 0.0f, 1.0f))));
    renderer.addUIElement(std::move(unique_ptr<UIElement>(new Slider("Scattering G",vec2(0.0f,0.0f), &fluidView->scatteringCoefficient.g, 0.0f, 1.0f))));
    renderer.addUIElement(std::move(unique_ptr<UIElement>(new Slider("Scattering B",vec2(0.0f,0.0f), &fluidView->scatteringCoefficient.b, 0.0f, 1.0f))));

    vec3 pos = fluidView->transform.getPosition();
    vec3 rot = fluidView->transform.getRotation();
    float frame = 1;
    float previousFrame = 0; // Track previous frame value


    renderer.addUIElement(std::move(unique_ptr<UIElement>(new Slider("Frame",vec2(0.0f,0.0f), &frame, 1.0f, 89.0f))));


    std::unique_ptr<float[]> densityField(new float[totalSize]());
    std::string filename = "Debug\\density_" + std::to_string(1) + ".bin";
    std::vector<float> vec = Utils::readGrid3D(filename.c_str(), Nx, Ny,Nz,dh);

        for (unsigned int k = 0; k < Nz; ++k) {
                for (unsigned int i = 0; i < Ny; ++i) {
                    for (unsigned int j = 0; j < Nx; ++j) {
                        unsigned int index = j + Nx * (i + Ny * k);
                        densityField.get()[index] = vec[index];
                    }
                }
            }
         // Update the fluid view
        fluidView->setDensityField(std::move(densityField), Nx, Ny, Nz);
        previousFrame = frame; // Initialize previous frame

    while(renderer.isRunning()){




    renderer.renderPass();

    
    if (static_cast<int>(frame) != static_cast<int>(previousFrame)) {
        std::unique_ptr<float[]> densityField(new float[totalSize]());
        std::string filename = "Debug\\density_" + std::to_string(int(std::floor(frame))) + ".bin";
        std::vector<float> vec = Utils::readGrid3D(filename.c_str(), Nx, Ny,Nz,dh);

        for (unsigned int k = 0; k < Nz; ++k) {
                for (unsigned int i = 0; i < Ny; ++i) {
                    for (unsigned int j = 0; j < Nx; ++j) {
                        unsigned int index = j + Nx * (i + Ny * k);
                        densityField.get()[index] = vec[index];
                    }
                }
            }
        
        fluidView->setDensityField(std::move(densityField), Nx, Ny, Nz);
        previousFrame = frame; // Update previous frame
        renderer.saveToFile(std::to_string(int(std::floor(frame))) + ".png",1024,768); //render
    }







        /*
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        

        //then this at the end of the render pass
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // NOW swap buffers //we needd to do that on the renderer, we will later incorporate it back into the render pass


        */


        
    }


    renderer.dispose();

    glfwTerminate();
    Log::closeLog();
    return 0;
}