#ifndef RENDERER_H
#define RENDERER_H
#include <map>
#include <vector>
#include <thread>
#include <chrono>
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
#include "InstancedModel.h"
#include "Volumetric.h"
#include "Utils.h"




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
    double mouseX,mouseY;




    Scene* loadedScene;
    bool hasSortedGroups;


    //stores all loaded shader
    std::map<ShaderType,unique_ptr<Shader>> loadedShaders;
    //Stores which all of the drawables that belongs to a given shader. Opaque object
    std::map<ShaderType, std::vector<shared_ptr<Drawable>>> opaqueShaderDrawGroups;
    std::vector<shared_ptr<Drawable>> transparentDrawGroups; //for transparent objects, we cant render by shader, only by their order of depth, since the drawable knows who is its shader, we can
                                                                  //get away with just a vector

    //Shader UBO's
    unsigned int gl_Matrixes_UBO;
    unsigned int gl_Camera_UBO;

    //skybox (render fixed for now)
    unsigned int gl_SkyBox_Cubemap;
    unsigned int gl_Skybox_VBO; 
    unsigned int gl_Skybox_VAO; 
    unique_ptr<Shader> skyboxShader;

    // shadow mapping stuff, this will turn into vector for multiple shadows
    mat4 worldToLightMat;
    unsigned int gl_ShadowMap_FBO; 
    unsigned int gl_ShadowMap_TEX;

    //multiple shadow casters
    unsigned int MAX_SHADOWCASTERS = 16;
    mat4 gl_LightMatrices[16];
    unsigned int gl_ShadowMap_FBOS[16]; 
    unsigned int gl_ShadowMap_TEX_ARRAY; //openGL has a texture array to work with, it has a layer type interface
    unsigned int activeShadowCasters;
    std::vector<shared_ptr<Light>> shadowCasters;
    unsigned int gl_ShadowMap_Resolution;
    unique_ptr<Shader> shadowMapShader;

    Shader* debugShader;





    //post processingg stuff
    unsigned int gl_ScreenQuad_VBO; //our vertex buffer
    unsigned int gl_ScreenQuad_VAO; //our vertex array

    unsigned int gl_Screen_FBO;
    unsigned int gl_Screen_Volumetric_FBO;


    unsigned int gl_Screen_TEX; //The texture that stores the colors
    unsigned int gl_Screen_DepthStencil_TEX; //the texture that stores depth and stencil values, complete

    unsigned int gl_Screen_Volumetric_TEX;
    unsigned int gl_Screen_Volumetric_DepthStencil_TEX; 

    unique_ptr<Shader> postProcessShader;
    

    



    //startup functions
    void loadShaders();
    void loadScreenBuffer();
    void loadSkyBox();
    void loadShadowMap();

    //Utility function, sorts every scene object in their group. We call this every frame but it is aware of updates
    //operations are done since we only need to sort groups once

    void sortSceneModels();

    //Input functions
    void processInput();


    //renering setup functions
    void setupShaders();
    void setupShaderLighting(Shader* shader);
    //Rendering functions
    void shadowPass();
    void geometryPass();


    //callbacks functions
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);









    
public:

    Renderer(unsigned int width, unsigned int height, const char* title);



    bool isRunning();
    void renderPass();
    void dispose();


    void loadScene(Scene* scene);



};
#endif
