#include "../headers/Renderer.h"
#include <vector>

Renderer::Renderer(unsigned int width, unsigned int height, const char* title) {

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //disable rezing until i fix the FBO resizin along the window and the camera aspect ratio


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
    //glEnable(GL_CULL_FACE);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND); 

    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    
    glCullFace(GL_BACK);



    // Initialize camera
    this->camera = std::unique_ptr<Camera>( new Camera(45.0f, (float)width / (float)height, 0.1f, 100.0f, width, height));
    this->camera->setPosition(vec3(0.0f, 0.0f, 5.0f));
    this->camera->setTarget(vec3(0.0f, 0.0f, 0.0f));


    this->loadShaders();
    this->loadScreenBuffer();
    this->loadSkyBox();





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

        //initialize the availables UBOS for shaders to bind to
        //now this requires some work
        /* //now we send the data, since we are using GL4.2+, the bind the point  is statically in the shader code, so we dont need to tell to each shader the binding point
        layout(std140, binding = 1) uniform Matrixes  {

        mat4 UBO_viewMat; =  4*4*N   = 64 bytes pos at offset 0
        mat4 UBO_projectionMat; 64 bytes at offset 64
        };  */
        //size is 128
        glGenBuffers(1, &this->gl_Matrixes_UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, this->gl_Matrixes_UBO);
        glBufferData(GL_UNIFORM_BUFFER, 128, NULL, GL_STATIC_DRAW); // allocate 152 bytes of memory
        glBindBufferRange(GL_UNIFORM_BUFFER, 0,this->gl_Matrixes_UBO, 0, 128); //binding the whole range of the buffer to binding point 0
        glBindBuffer(GL_UNIFORM_BUFFER,0);//unbind it

        //now for the camera UBO
        /*
         layout(std140, binding = 1) uniform CameraUBO  {
        vec3 cameraPos; //4N = 16 offset at 0
        vec3 cameraRot; //4N = 16 offset at 16
            };

        */
        glGenBuffers(1, &this->gl_Camera_UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, this->gl_Camera_UBO);
        glBufferData(GL_UNIFORM_BUFFER,32,NULL,GL_STATIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER,1,this->gl_Camera_UBO,0,32);
        glBindBuffer(GL_UNIFORM_BUFFER,0);



}

void Renderer::loadSkyBox(){
        std::vector<std::string> paths = {
            "Textures/Skybox/posx.jpg",  // GL_TEXTURE_CUBE_MAP_POSITIVE_X
            "Textures/Skybox/negx.jpg",  // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
            "Textures/Skybox/posy.jpg",  // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
            "Textures/Skybox/negy.jpg",  // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
            "Textures/Skybox/posz.jpg",  // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
            "Textures/Skybox/negz.jpg"   // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        };

    glGenTextures(1, &this->gl_SkyBox_Cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->gl_SkyBox_Cubemap);
    int width, height, nrChannels;
    //now the directry is fixed because i need to think how we will approach this, we also just load it once and bypass the texturehandler since this is different 
    for(int i = 0;i<6;i++){
            unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &nrChannels, 0);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0,GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);

    }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


        //now we create the vertex of the cube

        float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};


        glGenVertexArrays(1,&this->gl_Skybox_VAO);
        glGenBuffers(1,&this->gl_Skybox_VBO);
        glBindVertexArray(this->gl_Skybox_VAO);

        glBindBuffer(GL_ARRAY_BUFFER,this->gl_Skybox_VBO);
        glBufferData(GL_ARRAY_BUFFER,sizeof(skyboxVertices),skyboxVertices,GL_STATIC_DRAW);

        //for vertex attr, we dont use texture coords since we will map them using position
        glEnableVertexAttribArray(0);       //stride is zero because we only send one arg
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,0, (void*)0); // position
        

        this->skyboxShader = unique_ptr<Shader>(new Shader());
        if(!(this->skyboxShader->loadFromFile("Shaders/skybox_vertex.glsl","Shaders/skybox_frag.glsl"))){
            Log::write("[Renderer::loadSkybox] -  Failed to load the skybox shader!");
        }





}

void Renderer::loadScreenBuffer(){
    glGenVertexArrays(1, &this->gl_ScreenQuad_VAO);  
    glGenBuffers(1, &this->gl_ScreenQuad_VBO);      

    
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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); // position (its two because z is null it is always in zero)
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
   


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
    //Debug
    if(glfwGetKey(this->gl_Window,GLFW_KEY_W) ==  GLFW_PRESS){
        this->loadedScene->getLights()[LightType::POINT][0]->transform.setPosition( this->loadedScene->getLights()[LightType::POINT][0]->transform.getPosition() + vec3(0,5,0)*deltaTime);

    }
    if(glfwGetKey(this->gl_Window,GLFW_KEY_S) ==  GLFW_PRESS){
        this->loadedScene->getLights()[LightType::POINT][0]->transform.setPosition( this->loadedScene->getLights()[LightType::POINT][0]->transform.getPosition() + vec3(0,-5,0)*deltaTime);

    }

    if(glfwGetKey(this->gl_Window,GLFW_KEY_D) ==  GLFW_PRESS){
        this->loadedScene->getLights()[LightType::POINT][0]->transform.setPosition( this->loadedScene->getLights()[LightType::POINT][0]->transform.getPosition() + vec3(+5,0,0)*deltaTime);

    }
    if(glfwGetKey(this->gl_Window,GLFW_KEY_A) ==  GLFW_PRESS){
        this->loadedScene->getLights()[LightType::POINT][0]->transform.setPosition( this->loadedScene->getLights()[LightType::POINT][0]->transform.getPosition() + vec3(-5,0,0)*deltaTime);

    }



    this->camera->receiveInput(inputDir,this->deltaTime,zoomIn,zoomOut);
}





void Renderer::setupShaders(){
    for(int i = 0;i<SHADER_COUNT;i++){
        ShaderType type = (ShaderType)i;
        Shader* shader = nullptr;




        //now we set the actuall memory
        //matrix
        glBindBuffer(GL_UNIFORM_BUFFER,this->gl_Matrixes_UBO);
        glBufferSubData(GL_UNIFORM_BUFFER,0,sizeof(mat4),glm::value_ptr(this->camera->getViewMat()));
        glBufferSubData(GL_UNIFORM_BUFFER,64,sizeof(mat4),glm::value_ptr(this->camera->getProjectionMat()));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);  //unbind the buffer
        //cameraPos and roation for ease
        //since they are vec3, they need padding
        glBindBuffer(GL_UNIFORM_BUFFER,this->gl_Camera_UBO);
        glBufferSubData(GL_UNIFORM_BUFFER,0,sizeof(vec4),glm::value_ptr(this->camera->getPosition()));
        glBufferSubData(GL_UNIFORM_BUFFER,16,sizeof(vec4),glm::value_ptr(this->camera->getRotation())); //in degrees
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        //now the hard part with these buffers will be the lights, since we wil have 3 big vectors with stride things


        switch (type)
        {
        case ShaderType::Lit:
            shader = loadedShaders[type].get();
            loadedShaders[type]->bindShader();
            setupShaderLighting(shader);
            //also set the skybox for enviroment reflection //Careful HERE, IT WIL BIND AUTOMATICALLY TO TEXT UNITY 0! -> WE
            glBindTexture(GL_TEXTURE_CUBE_MAP, this->gl_SkyBox_Cubemap);
            loadedShaders[type]->setUniform("skybox",0);
            break;
        case ShaderType::Outline:
            shader = loadedShaders[type].get();
            loadedShaders[type]->bindShader();
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
        PointLight* light = static_cast<PointLight*>(this->loadedScene->getLights()[LightType::POINT][i].get());
        shader->setUniform("pointLights[" + std::to_string(i) +"].position", light->transform.getPosition());
        shader->setUniform("pointLights[" + std::to_string(i) +"].color",  light->getColor());
        shader->setUniform("pointLights[" + std::to_string(i) +"].intensity",  light->getIntensity());

    }

    for(int i = 0; i < this->loadedScene->getLights()[LightType::DIRECTIONAL].size(); i++){
        DirectionalLight* light = static_cast<DirectionalLight*>(this->loadedScene->getLights()[LightType::DIRECTIONAL][i].get());
        shader->setUniform("dirLights[" + std::to_string(i) +"].direction", light->transform.getForward());
        shader->setUniform("dirLights[" + std::to_string(i) +"].color", light->getColor());
        shader->setUniform("dirLights[" + std::to_string(i) +"].intensity", light->getIntensity());

    }
    for(int i = 0; i < this->loadedScene->getLights()[LightType::SPOT].size(); i++){
        SpotLight* light = static_cast<SpotLight*>(this->loadedScene->getLights()[LightType::SPOT][i].get());
        shader->setUniform("spotLights[" + std::to_string(i) +"].position", light->transform.getPosition());
        shader->setUniform("spotLights[" + std::to_string(i) +"].direction", light->transform.getForward());
        shader->setUniform("spotLights[" + std::to_string(i) +"].color", light->getColor());
        shader->setUniform("spotLights[" + std::to_string(i) +"].intensity",light->getIntensity());
        shader->setUniform("spotLights[" + std::to_string(i) +"].theta", light->getTheta());

    }
}

void Renderer::renderPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, this->gl_Screen_FBO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    // we need to change toa  shared VAO, since all meshes have the same attributes.
    //we can create one statically in a static funcion in mesh class and just get it here before rendering aall ehes

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


    
    //After that, we draw the skybox
    skyboxShader->bindShader();
    skyboxShader->setUniform("projectionMat",camera->getProjectionMat());
    skyboxShader->setUniform("viewMat",glm::mat4(glm::mat3(camera->getViewMat())));
    glBindVertexArray(this->gl_Skybox_VAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->gl_SkyBox_Cubemap);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    //Finally, we draw the transparent 

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
        //std::cout << 1/deltaTime << std::endl;
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