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
        Log::write("[RendererRenderer] - Failed to create GLFW this->gl_Window");
        glfwTerminate();
        return;
    }
    


    glfwMakeContextCurrent(this->gl_Window);
    glfwSetFramebufferSizeCallback(this->gl_Window, this->framebuffer_size_callback);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Log::write("[RendererRenderer] - Failed to initialize GLAD");
        glfwTerminate();
        return;
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


    this->loadShaders();
    this->loadScreenBuffer();





}

void Renderer::loadShaders(){
    for(int i = 0;i<SHADER_COUNT;i++){
        ShaderType type = (ShaderType)i;
        this->loadedShaders[type] = std::unique_ptr<Shader>(new Shader());
        if(type == ShaderType::Lit){
            if( !(this->loadedShaders[(ShaderType)i]->loadFromFile("Shaders/lit_vertex.glsl", "Shaders/lit_frag.glsl"))){
                Log::write("[RendererRenderer] - Failed to load lit shader");

            }
        }
        else if(type == ShaderType::Outline){
            if(!( this->loadedShaders[(ShaderType)i]->loadFromFile("Shaders/outline_vertex.glsl", "Shaders/outline_frag.glsl"))){
                Log::write("[RendererRenderer] - Failed to load outline");
            }
        }

    }


}

void Renderer::loadScreenBuffer(){
    glGenVertexArrays(1, &this->gl_ScreenQuad_VAO);  
    glGenBuffers(1, &this->gl_ScreenQuad_VBO);      // Fixed: was gl_ScreenQuad_VAO

    
    glBindVertexArray(this->gl_ScreenQuad_VAO);     // Fixed: was gl_ScreenQuad_VBO
    
    // Hardcoded coords in NDC to cover the whole screen
    float quadVertices[] = {  
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };  
    
    glBindBuffer(GL_ARRAY_BUFFER, this->gl_ScreenQuad_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); // position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); // tex coords
    
    // Load post processing shader
    this->postProcessShader = unique_ptr<Shader>(new Shader());
    this->postProcessShader->loadFromFile("PostProcessing/vertex_post.glsl","PostProcessing/fragment_post.glsl");
 
    // Generate and bind the framebuffer
    glGenFramebuffers(1, &this->gl_Screen_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, this->gl_Screen_FBO);  // Added: bind FBO
    
    // Generate and setup screen texture
    glGenTextures(1, &this->gl_Screen_TEX);
    glBindTexture(GL_TEXTURE_2D, this->gl_Screen_TEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->gl_Screen_TEX, 0);
   
    // Setup depth/stencil renderbuffer
    glGenRenderbuffers(1, &this->gl_Screen_RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, this->gl_Screen_RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->width, this->height);  
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->gl_Screen_RBO);
    
    // Check framebuffer completeness
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        Log::write("[Renderer::loadScreenBuffer] - Failed to initialize the Screen buffer");
    }
       
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind
    glBindRenderbuffer(GL_RENDERBUFFER, 0); // Added: unbind renderbuffer
}


void Renderer::sortSceneModels(){
 //we only do this once per scene unless we add more objects to the scene which we will check later using some flags
    if(this->hasSortedGroups == false){
        const auto& models = this->loadedScene->getModels();
        for(auto modelIt = models.begin(); modelIt != models.end(); ++modelIt){
            if((*modelIt)->getRenderGroup() == RenderGroup::Opaque){
               opaqueShaderDrawGroups[(*modelIt)->getShaderType()].push_back(*modelIt);
            }
            else if((*modelIt)->getRenderGroup() == RenderGroup::Transparent){
               transparentDrawGroups.push_back(*modelIt);
            }


       }
        this->hasSortedGroups = true;
    }
    //now for the transparent models, we will sort them by camera distance for the render, note that, we only do that if
    glm::vec3 camPos = camera->getPosition();

    // Sort back-to-front
    std::sort(transparentDrawGroups.begin(), transparentDrawGroups.end(),
        [&camPos](const std::shared_ptr<Drawable> a, const std::shared_ptr<Drawable> b) {
            Model* modelA;
            Model* modelB;
            if(a->getType() == DrawableType::MODEL && b->getType() == DrawableType::MODEL){
                modelA = static_cast<Model*>(a.get());
                modelB = static_cast<Model*>(b.get());

                float distA = glm::length2(camPos - modelA->transform.getPosition());
                float distB = glm::length2(camPos - modelB->transform.getPosition());
                return distA > distB; // farthest first
            }
            else{
                Log::write("[Renderer::sortSceneModels] -  WARNING! Transparent object without a transform was tried to be sorted!");
                return false;

            }
  

        }
    );










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
    
    // we need to change toa  shared VAO, since all meshes have the same attributes.

    //we can create one statically in a static funcion in mesh class and just get it here before rendering aall ehes


    glBindFramebuffer(GL_FRAMEBUFFER, this->gl_Screen_FBO);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    setupShaders();
    sortSceneModels();


    //render loop for the loaded scene
    //first we render all opaque objects, while storing transparents for later, outlines are broken because we need to render them after everythng
    std::vector<std::pair<ShaderType,Model*>> transparentModels; //can be optimized to out the loop
    for(int i = 0;i<loadedShaders.size();i++){
        for(auto it = opaqueShaderDrawGroups[(ShaderType) i].begin() ;it != opaqueShaderDrawGroups[(ShaderType) i].end();it++){
            Shader* shader = loadedShaders[(ShaderType) i].get();
            Drawable* drawable = it->get();
            if(drawable->getType() == DrawableType::MODEL){
                glStencilMask(0x00);
                Model* model = static_cast<Model*>(drawable); 
                
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
                    //Log::write("[Renderer::renderPass] - WARNING - OUTLINES ARE DISABLED UNTIL I SORT THEM CORRECTLY!");      
                                                        
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
        }
    }




    for (auto& drawable : this->transparentDrawGroups) {
        Shader* shader = loadedShaders[drawable->getShaderType()].get();
        if(drawable->getType() == DrawableType::MODEL){
            Model* model = static_cast<Model*>(drawable.get()); 
            shader->bindShader();
            shader->setUniform("modelMat",model->transform.getTransformMat());
            shader->setUniform("normalMat",model->transform.getNormalMat());
            model->draw(shader);
        }
        
        
    }

    //At this point, if everything is ok, we can now move on to the post processing 
    //bindd to the window buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glDisable(GL_DEPTH_TEST);
    
    // Activate texture unit 0 and bind our screen texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->gl_Screen_TEX);
    
    // Bind shader and set tex unity
    this->postProcessShader->bindShader();
    this->postProcessShader->setUniform("screenTexture", 0);
    

    glBindVertexArray(this->gl_ScreenQuad_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);



}

bool Renderer::isRunning() {
    return !glfwWindowShouldClose(this->gl_Window);
}

void Renderer::loop() {
    while (isRunning()) {
        

        this->currentFrame = static_cast<float>(glfwGetTime());
        this->deltaTime = this->currentFrame - this->lastFrame;
        this->lastFrame = this->currentFrame;
        std::cout << 1/deltaTime << std::endl;
        
        processInput();


        renderPass();

        glfwSwapBuffers(this->gl_Window);
        glfwPollEvents();
    }
}

void Renderer::dispose(){
    // Clean up resources
    delete this->loadedScene;




    glfwDestroyWindow(this->gl_Window);
    glfwTerminate();
}


void Renderer::loadScene(Scene* scene){
    this->loadedScene = scene;
    this->hasSortedGroups = false;

    sortSceneModels();

    //update all sorting layers
}

void Renderer::framebuffer_size_callback(GLFWwindow* gl_Window, int width, int height) {
    glViewport(0, 0, width, height);
}