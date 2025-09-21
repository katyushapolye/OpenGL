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
    glEnable(GL_BLEND); 

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    
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
    //first we render all opaque objects, while storing transparents for later, outlines are broken because we need to render them after everythng
    std::vector<std::pair<ShaderType,Model*>> transparentModels;
    for(int i = 0;i<loadedShaders.size();i++){
        for(auto it = shaderModelGroups[(ShaderType) i].begin() ;it != shaderModelGroups[(ShaderType) i].end();it++){
            auto& shader = loadedShaders[(ShaderType) i];
            Drawable* drawable = it->get();

            if(drawable->getType() == Drawable::DrawableType::MODEL){
                glStencilMask(0x00);
                Model* model = static_cast<Model*>(drawable); 
                if(model->getRenderGroup() == RenderGroup::Opaque){
                    shader->bindShader();
                    shader->setUniform("modelMat",model->transform.getTransformMat());
                    shader->setUniform("normalMat",model->transform.getNormalMat());
                    if(model->hasOutline == false){
                        model->draw(shader);
                    }
                    else{
                        //glStencilMask(0xFF);  // enable stencil writes
                        ////we will use the stencil buffer for some neat things
                        //glStencilFunc(GL_ALWAYS, 1, 0xFF);        // always pass
                        //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // replace stencil with 1
                                                                         
                                                                // keep depth for original model
                        model->draw(shader);  
                        Log::write("[Renderer::renderPass] - WARNING - OUTLINES ARE DISABLED UNTIL I SORT THEM CORRECTLY!");      
                                                            
                        //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                        //glStencilMask(0x00); 
                        //glDisable(GL_DEPTH_TEST);
                        //loadedShaders[ShaderType::Outline]->bindShader();
                        //shader->setUniform("modelMat",model->transform.getTransformMat());
                        //shader->setUniform("normalMat",model->transform.getNormalMat());
                        //model->draw(shader);
                        //glStencilMask(0xFF);
                        //glStencilFunc(GL_ALWAYS, 1, 0xFF);   
                        //glEnable(GL_DEPTH_TEST);  







                    }
                }
                //transparent model
                else if(model->getRenderGroup() == RenderGroup::Transparent){
                    transparentModels.push_back(std::pair<ShaderType,Model*>((ShaderType) i,model));
                }

            }


            //setup the model matrix and finally render it

        }
    }

    glm::vec3 camPos = camera->getPosition();

    // Sort back-to-front
    std::sort(transparentModels.begin(), transparentModels.end(),
        [&camPos](const std::pair<ShaderType, Model*>& a, const std::pair<ShaderType, Model*>& b) {
            float distA = glm::length2(camPos - a.second->transform.getPosition());
            float distB = glm::length2(camPos - b.second->transform.getPosition());
  
            return distA > distB; // farthest first
        }
    );


    for (auto& entry : transparentModels) {
        
        loadedShaders[entry.first]->bindShader();
        loadedShaders[entry.first]->setUniform("modelMat",entry.second->transform.getTransformMat());
        loadedShaders[entry.first]->setUniform("normalMat",entry.second->transform.getNormalMat());
        entry.second->draw(loadedShaders[entry.first]);
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