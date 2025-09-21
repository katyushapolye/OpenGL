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

    Model* axisGizmo;

    Model* testModel;

    std::unique_ptr<Shader> testShader;
    std::vector<Material>  testMaterials;


    glm::mat3 testNormalMatrix;


    std::map<LightType,std::vector<std::shared_ptr<Light>>> loadedLights;

    void computeNormalsMatrixes();

    void lightningPass();

    void processInput();
    void renderPass();

    //callbacks functions
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);


    void debug_INIT();
    void debug_LOOP();








    
public:
    Renderer(unsigned int width, unsigned int height, const char* title);



    bool isRunning();

    void loop();

    void dispose();



};
#endif

