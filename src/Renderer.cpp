#include "../headers/Renderer.h"
#include <vector>

Renderer::Renderer(unsigned int width, unsigned int height, const char* title) {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    this->width = width;
    this->height = height;
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
    glEnable(GL_STENCIL_TEST);

    glCullFace(GL_BACK);



    // Initialize camera
    this->camera = std::unique_ptr<Camera>( new Camera(45.0f, (float)width / (float)height, 0.1f, 100.0f, width, height));
    this->camera->setPosition(vec3(0.0f, 0.0f, 5.0f));
    this->camera->setTarget(vec3(0.0f, 0.0f, 0.0f));

    //Initialize the shaders
    this->testShader = std::unique_ptr<Shader>(new Shader());
    this->testShader->loadFromFile("Shaders/lit_vertex.glsl", "Shaders/lit_frag.glsl");

    loadShaders();

}

void Renderer::loadShaders(){
    for(int i = 0;i<SHADER_COUNT;i++){
        ShaderType type = (ShaderType)i;
        this->loadedShaders[type] = std::unique_ptr<Shader>(new Shader());
        if(type == ShaderType::Lit){
            this->loadedShaders[(ShaderType)i]->loadFromFile("Shaders/lit_vertex.glsl", "Shaders/lit_frag.glsl");
        }
        if(type == ShaderType::Outline){
            this->loadedShaders[(ShaderType)i]->loadFromFile("Shaders/outline_vertex.glsl", "Shaders/outline_frag.glsl");
        }

    }


}

void Renderer::sortSceneModels(){
 const auto& models = this->loadedScene->getModels();
    for(auto modelIt = models.begin(); modelIt != models.end(); ++modelIt){
        shaderModelGroups[(*modelIt)->getShaderType()].push_back(*modelIt);

    }




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





void Renderer::setupShaders(){
    for(int i = 0;i<SHADER_COUNT;i++){
        ShaderType type = (ShaderType)i;
        Shader* shader = nullptr;

        switch (type)
        {
        case ShaderType::Lit:
            shader = loadedShaders[type].get();
            loadedShaders[type]->bindShader();
            loadedShaders[type]->setUniform("viewMat", this->camera->getViewMat());
            loadedShaders[type]->setUniform("projectionMat", this->camera->getProjectionMat());
            loadedShaders[type]->setUniform("camera.position", this->camera->getPosition());
            //this shader is lit so we set its lights
            setupShaderLighting(shader);
            break;
        case ShaderType::Outline:
            shader = loadedShaders[type].get();
            loadedShaders[type]->bindShader();
            loadedShaders[type]->setUniform("viewMat", this->camera->getViewMat());
            loadedShaders[type]->setUniform("projectionMat", this->camera->getProjectionMat());
            loadedShaders[type]->setUniform("camera.position", this->camera->getPosition());
            //this shader is lit so we set its lights
            setupShaderLighting(shader);
            break;
        
        default:
            break;
        }

        
    }


}

void Renderer::setupShaderLighting(Shader* shader){



    shader->setUniform("ambientLight",this->loadedScene->ambientLight);

    for(int i = 0; i < this->loadedScene->getLights()[LightType::POINT].size(); i++){
        shader->setUniform("pointLights[" + std::to_string(i) +"].position", this->loadedScene->getLights()[LightType::POINT][i]->transform.getPosition());
        shader->setUniform("pointLights[" + std::to_string(i) +"].color", this->loadedScene->getLights()[LightType::POINT][i]->color);
        shader->setUniform("pointLights[" + std::to_string(i) +"].intensity", this->loadedScene->getLights()[LightType::POINT][i]->intensity);

    }

    for(int i = 0; i < this->loadedScene->getLights()[LightType::DIRECTIONAL].size(); i++){
        shader->setUniform("dirLights[" + std::to_string(i) +"].direction", this->loadedScene->getLights()[LightType::DIRECTIONAL][i]->transform.getForward());
        shader->setUniform("dirLights[" + std::to_string(i) +"].color", this->loadedScene->getLights()[LightType::DIRECTIONAL][i]->color);
        shader->setUniform("dirLights[" + std::to_string(i) +"].intensity", this->loadedScene->getLights()[LightType::DIRECTIONAL][i]->intensity);

    }
    for(int i = 0; i < this->loadedScene->getLights()[LightType::SPOT].size(); i++){
        shader->setUniform("spotLights[" + std::to_string(i) +"].position", this->loadedScene->getLights()[LightType::SPOT][i]->transform.getPosition());
        shader->setUniform("spotLights[" + std::to_string(i) +"].direction", this->loadedScene->getLights()[LightType::SPOT][i]->transform.getForward());
        shader->setUniform("spotLights[" + std::to_string(i) +"].color", this->loadedScene->getLights()[LightType::SPOT][i]->color);
        shader->setUniform("spotLights[" + std::to_string(i) +"].intensity",this->loadedScene->getLights()[LightType::SPOT][i]->intensity);
        shader->setUniform("spotLights[" + std::to_string(i) +"].theta", this->loadedScene->getLights()[LightType::SPOT][i]->theta);

    }
}

void Renderer::renderPass() {
    
    // Clear color and depth buffers


    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //precompute normals
    setupShaders();


    //render loop for the loaded scene
    //we will need to also sort by z-depth here later
    for(int i = 0;i<loadedShaders.size();i++){
        for(auto it = shaderModelGroups[(ShaderType) i].begin() ;it != shaderModelGroups[(ShaderType) i].end();it++){
            auto& shader = loadedShaders[(ShaderType) i];
            Drawable* drawable = it->get();

            if(drawable->getType() == Drawable::DrawableType::MODEL){
                glStencilMask(0x00);
                Model* model = static_cast<Model*>(drawable); 
                shader->bindShader();
                shader->setUniform("modelMat",model->transform.getTransformMat());
                shader->setUniform("normalMat",model->transform.getNormalMat());

                if(model->hasOutline == false){
                    model->draw(shader);
                }
                else{
                    
                    //we will use the stencil buffer for some neat things
                    glStencilMask(0xff); //enable writes at this point all of our mask is zero
                    glStencilFunc(GL_ALWAYS,1,0xff); //we always pass the test our reference is zero
                    glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE); //we will only increment the mask if the drawn frag is in visible, so now our object frag area has ones
                    //draw our object
                    model->draw(shader);

                    //glStencilMask(0x00); //disable writes
                    glStencilFunc(GL_NOTEQUAL,1,0xff); //we now say our stencil only passes if it is neq
                    glDisable(GL_DEPTH_TEST);     //we also disabel depth test becayse we want it ot be draw always
                    ////now we use our outline shader, which scales the object a bit and draws a solid color, since we wont draw on the previous marked frags 
                    loadedShaders[ShaderType::Outline]->bindShader();
                    loadedShaders[ShaderType::Outline]->setUniform("modelMat",model->transform.getTransformMat());
                    loadedShaders[ShaderType::Outline]->setUniform("normalMat",model->transform.getNormalMat());
                    model->draw(loadedShaders[ShaderType::Outline]);
                    glEnable(GL_DEPTH_TEST);
                    //glStencilMask(0xff);





                }
            }

            //setup the model matrix and finally render it

        }
    }



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


void Renderer::loadScene(Scene* scene){
    this->loadedScene = scene;

    sortSceneModels();

    //update all sorting layers
}

void Renderer::framebuffer_size_callback(GLFWwindow* gl_Window, int width, int height) {
    glViewport(0, 0, width, height);
}