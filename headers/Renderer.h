#ifndef RENDERER_H
#define RENDERER_H
#include <map>
#include <vector>
#include "Definitions.h"
#include "Camera.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Transform.h"
#include "Material.h"
#include "Light.h"
#include "Model.h"
#include "ModelLoader.h"
#include "Scene.h"




//simplified render for now
//since we dont have proper materials and object with meshes and textures and transforms linked
//we will just make a simple render class that will handle the render loop and the window
class Renderer
{
private:
    std::unique_ptr<Camera> camera;
    GLFWwindow* gl_Window;

    unsigned int width;
    unsigned int height;

    float deltaTime;
    float lastFrame;
    float currentFrame;




    Scene* loadedScene;

    //stores all loaded shader
    std::map<ShaderType,std::unique_ptr<Shader>> loadedShaders;
    //Stores which all of the drawables that belongs to a given shader.
    std::map<ShaderType, std::vector<std::shared_ptr<Drawable>>> shaderDrawGroups;





    //startup functions
    void loadShaders();

    //updates the sorting layes of shaders
    void sortSceneModels();





    void processInput();


    void renderPass();
    void setupShaders();
    void setupShaderLighting(Shader* shader);

    //callbacks functions
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);









    
public:

    Renderer(unsigned int width, unsigned int height, const char* title);



    bool isRunning();
    void loop();
    void dispose();


    void loadScene(Scene* scene);



};
#endif
