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
    fluidView->scatteringCoefficient = vec3(0.2, 0.175, 0.1);
    scene->addModel(shared_ptr<Volumetric>(fluidView));

    renderer.loadScene(scene);

    int Nx = 64, Ny = 64, Nz = 64;
    float dh = 1.0/64.0;
    size_t totalSize = Nx * Ny * Nz;

    std::unique_ptr<float[]> densityField(new float[totalSize]());

    int currentFrame = 3;
    const int maxFrame = 3;

    // ImGui slider variable
    float densityOffset = 250.0f;

    while(renderer.isRunning()){

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create ImGui window with slider
        ImGui::Begin("HUD");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
        //std::cout << "density offset: " << densityOffset << std::endl;


        std::string filename = "Debug\\density_grid_"  +std::to_string(currentFrame) + ".bin";
        std::vector<float> vec = Utils::readGrid3D_Binary(filename.c_str(), Nx, Ny,Nz,dh);

        // Fill the density field
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

        renderer.renderPass();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // NOW swap buffers //we needd to do that on the renderer, we will later incorporate it back into the render pass
        glfwSwapBuffers(renderer.getWindow());
        glfwPollEvents();

        currentFrame++;
        if (currentFrame > maxFrame) {
            currentFrame = 3;
        }


        densityField.reset(new float[totalSize]());

        std::this_thread::sleep_for(33ms);
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    renderer.dispose();

    glfwTerminate();
    Log::closeLog();
    return 0;
}