#include "../headers/Renderer.h"
#include <vector>

Renderer::Renderer(unsigned int width, unsigned int height, const char* title) {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    this->width = width;
    this->height = height;
    this->testNormalMatrix = glm::mat3(1.0f);
    // Create this->gl_Window
    this->gl_Window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!this->gl_Window) {
        std::cerr << "Failed to create GLFW this->gl_Window" << std::endl;
        glfwTerminate();
        return;
    }


    glfwMakeContextCurrent(this->gl_Window);
    glfwSetFramebufferSizeCallback(this->gl_Window, this->framebuffer_size_callback);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        
    }


    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);


    // Initialize camera
    this->camera = std::unique_ptr<Camera>( new Camera(45.0f, (float)width / (float)height, 0.1f, 100.0f, width, height));
    this->camera->setPosition(vec3(0.0f, 0.0f, 5.0f));
    this->camera->setTarget(vec3(0.0f, 0.0f, 0.0f));

    // Debug initialization (load test shader, texture, mesh)
    debug_INIT();
}

void Renderer::processInput(){
    vec2 inputDir = vec2(0.0f,0.0f);
    bool zoomIn = false;
    bool zoomOut = false;
    if (glfwGetKey(this->gl_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(this->gl_Window, true);

    if(glfwGetKey(this->gl_Window,GLFW_KEY_UP) ==  GLFW_PRESS){
        inputDir.y +=1.0f;

    }
    if(glfwGetKey(this->gl_Window,GLFW_KEY_DOWN) ==  GLFW_PRESS){
        inputDir.y -=1.0f;

    }
    if(glfwGetKey(this->gl_Window,GLFW_KEY_LEFT) ==  GLFW_PRESS){
        inputDir.x -=1.0f;

    }
    if(glfwGetKey(this->gl_Window,GLFW_KEY_RIGHT) ==  GLFW_PRESS){
        inputDir.x +=1.0f;

    }

    if(glfwGetKey(this->gl_Window,GLFW_KEY_M) ==  GLFW_PRESS){
        zoomIn = true;

    }
    if(glfwGetKey(this->gl_Window,GLFW_KEY_N) ==  GLFW_PRESS){
        zoomOut = true;

    }

    this->camera->receiveInput(inputDir,this->deltaTime,zoomIn,zoomOut);
}


void Renderer::computeNormalsMatrixes(){
    //for each transform...
    this->testNormalMatrix = glm::mat3(glm::transpose(glm::inverse(this->testModel->transform.getTransformMat())));


}

void Renderer::lightningPass(){
    this->testShader->bindShader();
    //point first
    this->testShader->setUniform("ambientLight",vec3(0.3,0.3,0.3));

    for(int i = 0; i < this->loadedLights[LightType::POINT].size(); i++){
        this->testShader->setUniform("pointLights[" + std::to_string(i) +"].position", this->loadedLights[LightType::POINT][i]->transform.getPosition());
        this->testShader->setUniform("pointLights[" + std::to_string(i) +"].color", this->loadedLights[LightType::POINT][i]->color);
        this->testShader->setUniform("pointLights[" + std::to_string(i) +"].intensity", this->loadedLights[LightType::POINT][i]->intensity);

    }

    for(int i = 0; i < this->loadedLights[LightType::DIRECTIONAL].size(); i++){
        this->testShader->setUniform("dirLights[" + std::to_string(i) +"].direction", this->loadedLights[LightType::DIRECTIONAL][i]->transform.getForward());
        this->testShader->setUniform("dirLights[" + std::to_string(i) +"].color", this->loadedLights[LightType::DIRECTIONAL][i]->color);
        this->testShader->setUniform("dirLights[" + std::to_string(i) +"].intensity", this->loadedLights[LightType::DIRECTIONAL][i]->intensity);

    }
    for(int i = 0; i < this->loadedLights[LightType::SPOT].size(); i++){
        this->testShader->setUniform("spotLights[" + std::to_string(i) +"].position", this->loadedLights[LightType::SPOT][i]->transform.getPosition());
        this->testShader->setUniform("spotLights[" + std::to_string(i) +"].direction", this->loadedLights[LightType::SPOT][i]->transform.getForward());
        this->testShader->setUniform("spotLights[" + std::to_string(i) +"].color", this->loadedLights[LightType::SPOT][i]->color);
        this->testShader->setUniform("spotLights[" + std::to_string(i) +"].intensity", this->loadedLights[LightType::SPOT][i]->intensity);
        this->testShader->setUniform("spotLights[" + std::to_string(i) +"].theta", this->loadedLights[LightType::SPOT][i]->theta);

    }
}

void Renderer::renderPass() {
    
    // Clear color and depth buffers
    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //precompute normals
    computeNormalsMatrixes();

    // Use test shader, wewill use UBOS later
    this->testShader->bindShader();
    this->testShader->setUniform("viewMat", this->camera->getViewMat());
    this->testShader->setUniform("projectionMat", this->camera->getProjectionMat());
    this->testShader->setUniform("normalMat", this->testNormalMatrix);
    this->testShader->setUniform("camera.position", this->camera->getPosition());

    lightningPass();


    
 



    this->testShader->bindShader();
    this->testShader->setUniform("modelMat", this->testModel->transform.getTransformMat());
    this->testModel->draw( this->testShader);

    this->testShader->setUniform("modelMat", this->axisGizmo->transform.getTransformMat());
    this->axisGizmo->draw(this->testShader);



}

bool Renderer::isRunning() {
    return !glfwWindowShouldClose(this->gl_Window);
}

void Renderer::loop() {
    while (isRunning()) {
        

        this->currentFrame = static_cast<float>(glfwGetTime());
        this->deltaTime = this->currentFrame - this->lastFrame;
        this->lastFrame = this->currentFrame;
        
        processInput();
        debug_LOOP();

        renderPass();
        glfwSwapBuffers(this->gl_Window);
        glfwPollEvents();
    }
}

void Renderer::dispose(){
    // Clean up resources




    glfwDestroyWindow(this->gl_Window);
    glfwTerminate();
}

void Renderer::debug_LOOP(){
    //spin the transfor
    //this->testTransform->rotateLocal(vec3(0.0f,50.0f*this->deltaTime,0.0f));
    //rotate the light across the center

    //this->loadedLights[LightType::SPOT][0]->transform.setPosition(vec3(0.0, 3, 0.0));

    //this->testModel->transform.rotateLocal(vec3(0.0f,0.0f,0.0f));
    //this->loadedLights[LightType::POINT][0]->transform.setPosition(vec3(lightX, 1.0, lightZ));
    //
    //this->loadedLights[LightType::DIRECTIONAL][0]->transform.setPosition(vec3(0.0, 1.0, 0.0));

   // std::cout << this->testTransform->getRotation().x << "," << this->testTransform->getRotation().y << "," << this->testTransform->getRotation().z << std::endl;
   //std::cout << "Loaed Count: " << TextureHandler::getLoadedTextureCount() << std::endl;
}

void Renderer::debug_INIT() {
    // Load test shader
    this->testShader = std::unique_ptr<Shader>(new Shader());
    this->testShader->loadFromFile("Shaders/lit_vertex.glsl", "Shaders/lit_frag.glsl");


    this->testModel =       ModelLoader::loadFromObjWithAssimp("Models/surface.obj");
    this->axisGizmo =       ModelLoader::loadFromObjWithAssimp("Models/gizmo.obj");

    //ModelLoader::loadFromObjWithAssimp("Models/surface.obj");

    
    loadedLights[LightType::DIRECTIONAL].reserve(4);
    loadedLights[LightType::POINT].reserve(16);
    loadedLights[LightType::SPOT].reserve(16);
    

    //loadedLights[LightType::POINT].push_back(std::shared_ptr<Light>(new Light(LightType::POINT, vec3(1.0f, 1.0f, 1.0f), 10.0f)));
    //loadedLights[LightType::POINT][0]->transform.setPosition(vec3(0.0,2.0f,1.5f));
    loadedLights[LightType::DIRECTIONAL].push_back(std::shared_ptr<Light>(new Light(LightType::DIRECTIONAL, vec3(1.0f, 1.0f, 1.0f), 1.0f)));
    //
    this->loadedLights[LightType::DIRECTIONAL][0]->transform.setRotation(vec3(135.0f,0.00f, 0.0f));

    //loadedLights[LightType::SPOT].push_back(std::shared_ptr<Light>(new Light(LightType::SPOT, vec3(1.0f, 1.0f,1.0f),10.0f)));
    //this->loadedLights[LightType::SPOT][0]->transform.setPosition(vec3(0.0, 3, 0.0));
    //loadedLights[LightType::SPOT][0]->theta = 30.0f;
    //loadedLights[LightType::SPOT][0]->transform.setPosition(vec3(0.0f, 1.0f, 0.0f));
    //loadedLights[LightType::SPOT][0]->transform.rotateGlobal(vec3(90.0f, 0.0f, 0.0f));


    
    
}

void Renderer::framebuffer_size_callback(GLFWwindow* gl_Window, int width, int height) {
    glViewport(0, 0, width, height);
}