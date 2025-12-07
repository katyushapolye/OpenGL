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

    scene->addModel(shared_ptr<Model>(ModelLoader::loadFromObj("Resources/Models/gizmo.obj")));

    m = ModelLoader::loadFromObj("Resources/Models/cube.obj");
    m->transform.setPosition(vec3(-1,10,0));
    m->transform.setScale(vec3(0.2,0.2,0.2));
    m->transform.rotateGlobal(vec3(0,0,45));
    scene->addModel(shared_ptr<Model>(m));

    Transform t = Transform();
    t.setPosition(vec3(0,5,0));


    
    Volumetric* fluidView = new Volumetric(t,10,10,10);
    fluidView->transform.rotateGlobal(vec3(0,0,90));
    fluidView->scatteringCoefficient = vec3(0.2, 0.175, 0.1);
    scene->addModel(shared_ptr<Volumetric>(fluidView));

    renderer.loadScene(scene);

    int Nx = 60, Ny = 60, Nz = 60;
    float dh = 1.0/60.0;
    size_t totalSize = Nx * Ny * Nz;

    std::unique_ptr<float[]> densityField(new float[totalSize]());



    // ImGui slider variable
    float densityOffset = 0.0f;

    renderer.addUIElement(std::move(unique_ptr<UIElement>(new Text("Hello world!",vec2(0.0f,0.0f)))));
    
    std::string filename = "Debug\\density.bin";
    std::vector<float> vec = Utils::readGrid3D(filename.c_str(), Nx, Ny,Nz,dh);

    for (unsigned int k = 0; k < Nz; ++k) {
            for (unsigned int i = 0; i < Ny; ++i) {
                for (unsigned int j = 0; j < Nx; ++j) {
                    unsigned int index = j + Nx * (i + Ny * k);
                    densityField.get()[index] = vec[index];
                    if(densityField.get()[index] > 0.001){
                        densityField.get()[index] += densityOffset;
                    }
                }
            }
        }
     // Update the fluid view
    fluidView->setDensityField(std::move(densityField), Nx, Ny, Nz);
    while(renderer.isRunning()){



        renderer.renderPass();

        /*
        first this
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