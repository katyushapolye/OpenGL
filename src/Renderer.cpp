#include "../headers/Renderer.h"
#include <vector>

/*
    For making some shadow system
    -make each light have its matrixes (via a function just like the camera)
    -we will batch all the matrixes in the shaders using UBOS
    //lets test first with only directional
*/

Renderer::Renderer(unsigned int width, unsigned int height, const char* title) {

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_SAMPLES, 4);
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
    glfwSetInputMode(this->gl_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //locks tthe mouse to screen
    if (glfwRawMouseMotionSupported()){
        Log::write("[RendererRenderer] - Mouse Raw acceleration is supported, turning it on");
        glfwSetInputMode(this->gl_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    glfwGetCursorPos(this->gl_Window,&this->mouseX,&this->mouseY);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Log::write("[RendererRenderer] - Failed to initialize GLAD");
        glfwTerminate();
        return;
    }
    
 

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);;
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND); 


    

    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    
    glCullFace(GL_BACK);



    // Initialize camera
    this->camera = std::unique_ptr<Camera>( new Camera(70.0f, (float)width / (float)height, 0.1f, 1000.0f, width, height));
    this->camera->setPosition(vec3(0.0f, 3.0f, 3.0f));
    this->camera->setRotation(vec3(-45.0f,75.0f,0.0f));
   //this->camera->lookAt(vec3(0,0,0));


    this->loadShaders();
    this->loadScreenBuffer();
    this->loadSkyBox();
    this->loadShadowMap();

    /*debug stuff*/

    debugShader = new Shader();

    debugShader->loadFromFile(
        "Resources/Shaders/Debug/normal_vert.glsl",
        "Resources/Shaders/Debug/normal_geo.glsl",
        "Resources/Shaders/Debug/normal_frag.glsl"
    );





}

void Renderer::loadShaders(){
    for(int i = 0;i<SHADER_COUNT;i++){
        ShaderType type = (ShaderType)i;
        this->loadedShaders[type] = std::unique_ptr<Shader>(new Shader());
        if(type == ShaderType::Lit){
            if( !(this->loadedShaders[(ShaderType)i]->loadFromFile("Resources/Shaders/lit_vertex.glsl", "Resources/Shaders/lit_frag.glsl"))){
                Log::write("[RendererRenderer] - Failed to load lit shader");

            }
        }
        else if(type == ShaderType::Instanced){
            if(!( this->loadedShaders[(ShaderType)i]->loadFromFile("Resources/Shaders/instanced_vertex.glsl", "Resources/Shaders/instanced_frag.glsl"))){
                Log::write("[RendererRenderer] - Failed to load instanced shader");
            }
        }
        else if(type == ShaderType::Raymarch){
            if(!( this->loadedShaders[(ShaderType)i]->loadFromFile("Resources/Shaders/volumetric_vertex.glsl", "Resources/Shaders/volumetric_frag.glsl"))){
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
        float nearPlane;
        float farPlane;
            };

        */
        glGenBuffers(1, &this->gl_Camera_UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, this->gl_Camera_UBO);
        glBufferData(GL_UNIFORM_BUFFER,2*sizeof(mat4) + sizeof(vec2),NULL,GL_STATIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER,1,this->gl_Camera_UBO,0,2*sizeof(mat4) + sizeof(vec2));
        glBindBuffer(GL_UNIFORM_BUFFER,0);



}

void Renderer::loadSkyBox(){
        std::vector<std::string> paths = {
            "Resources/Textures/Skybox/Sunny/posx.png",  // GL_TEXTURE_CUBE_MAP_POSITIVE_X
            "Resources/Textures/Skybox/Sunny/negx.png",  // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
            "Resources/Textures/Skybox/Sunny/posy.png",  // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
            "Resources/Textures/Skybox/Sunny/negy.png",  // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
            "Resources/Textures/Skybox/Sunny/posz.png",  // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
            "Resources/Textures/Skybox/Sunny/negz.png"   // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        };

    glGenTextures(1, &this->gl_SkyBox_Cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->gl_SkyBox_Cubemap);
    int width, height, nrChannels;
    //now the directry is fixed because i need to think how we will approach this, we also just load it once and bypass the texturehandler since this is different 
    for(int i = 0;i<6;i++){
            unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &nrChannels, 0);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0,GL_RGBA, GL_UNSIGNED_BYTE, data);
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
        if(!(this->skyboxShader->loadFromFile("Resources/Shaders/skybox_vertex.glsl","Resources/Shaders/skybox_frag.glsl"))){
            Log::write("[Renderer::loadSkybox] -  Failed to load the skybox shader!");
        }





}

void Renderer::loadScreenBuffer(){
    glGenVertexArrays(1, &this->gl_ScreenQuad_VAO);  
    glGenBuffers(1, &this->gl_ScreenQuad_VBO);      
   
    glBindVertexArray(this->gl_ScreenQuad_VAO);
   
    float quadVertices[] = {  
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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
   
    this->postProcessShader = unique_ptr<Shader>(new Shader());
    this->postProcessShader->loadFromFile("Resources/PostProcessing/vertex_post.glsl","Resources/PostProcessing/fragment_post.glsl");
 
    // Create framebuffer
    glGenFramebuffers(1, &this->gl_Screen_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, this->gl_Screen_FBO);
    
    // Create color texture
    glGenTextures(1, &this->gl_Screen_TEX);
    glBindTexture(GL_TEXTURE_2D, this->gl_Screen_TEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  // ADD THIS
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  // ADD THIS
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // ADD THIS
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  // ADD THIS
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->gl_Screen_TEX, 0);
   
    // Create depth/stencil texture 
    glGenTextures(1, &this->gl_Screen_DepthStencil_TEX);
    glBindTexture(GL_TEXTURE_2D, this->gl_Screen_DepthStencil_TEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, this->width, this->height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, this->gl_Screen_DepthStencil_TEX, 0);  

    //for buffer ping pong, we will use create another one exactly the same
    glGenFramebuffers(1, &this->gl_Screen_Volumetric_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, this->gl_Screen_Volumetric_FBO);

    // Create color texture
    glGenTextures(1, &this->gl_Screen_Volumetric_TEX);
    glBindTexture(GL_TEXTURE_2D, this->gl_Screen_Volumetric_TEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  // ADD THIS
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  // ADD THIS
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // ADD THIS
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  // ADD THIS
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->gl_Screen_Volumetric_TEX, 0);
   
    // Create depth/stencil texture 
    glGenTextures(1, &this->gl_Screen_Volumetric_DepthStencil_TEX);
    glBindTexture(GL_TEXTURE_2D, this->gl_Screen_Volumetric_DepthStencil_TEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, this->width, this->height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, this->gl_Screen_Volumetric_DepthStencil_TEX, 0);  


   
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        Log::write("[Renderer::loadScreenBuffer] - Failed to initialize the Screen buffer");
    }
       
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void Renderer::loadShadowMap() {
    

    //multiple maps actual implementation
    this->gl_ShadowMap_Resolution = 4096;
    
    glGenFramebuffers(this->MAX_SHADOWCASTERS, this->gl_ShadowMap_FBOS);
    //generate 32 textures for the maps
    
    glGenTextures(1,&this->gl_ShadowMap_TEX_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY,this->gl_ShadowMap_TEX_ARRAY);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY,1,GL_DEPTH_COMPONENT32 ,this->gl_ShadowMap_Resolution
    ,this->gl_ShadowMap_Resolution,this->MAX_SHADOWCASTERS);
    
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);


    //now we go binding all of the texs to their respective framebuffers
    for(int i = 0;i<this->MAX_SHADOWCASTERS;i++){
        glBindFramebuffer(GL_FRAMEBUFFER,this->gl_ShadowMap_FBOS[i]);
        //0 is the mipmap and i is the layer
        glFramebufferTextureLayer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,this->gl_ShadowMap_TEX_ARRAY,0,i);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE); //we are not using color for the shadow
    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);   
    glReadBuffer(GL_BACK);
    
    
    this->shadowMapShader = unique_ptr<Shader>(new Shader());
    if(!(this->shadowMapShader->loadFromFile("Resources/Shaders/shadow_vertex.glsl",
                                              "Resources/Shaders/shadow_frag.glsl"))) {
        Log::write("[Renderer::loadShadowMap] - Failed to load shadowmapping shader!");
    }




    
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
    //this is a big issue because we have volumetric effects
    glm::vec3 camPos = camera->getPosition();
    //std::cout << "s: "  << transparentDrawGroups.size() << std::endl;


    // Sort back-to-front
   std::sort(transparentDrawGroups.begin(), transparentDrawGroups.end(),
    [&camPos](const std::shared_ptr<Drawable>& a, const std::shared_ptr<Drawable>& b) {
        return Utils::compareDrawablesFarthestFirst(a, b, camPos);
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
    //mouse
    vec2 mouseDir;
    double x,y;
    
    glfwGetCursorPos(this->gl_Window,&x,&y);

    mouseDir.x = ( x-this->mouseX) / this->deltaTime;
    mouseDir.y = ( y-this->mouseY) / this->deltaTime;
    this->mouseX = x;
    this->mouseY = y;



    this->camera->receiveInput(inputDir,mouseDir,this->deltaTime,zoomIn,zoomOut);
    DirectionalLight* light = (DirectionalLight*)this->loadedScene->getLights()[LightType::DIRECTIONAL][0].get();

    //light->transform.setPosition(this->camera->getPosition()  - light->transform.getUp()); //the shadows dont appear
    //light->transform.lookAt(this->camera->getPosition() + 10.0f*this->camera->getForward());
    
 
    //if i move the light here manually it is ok
    

    if(glfwGetKey(this->gl_Window,GLFW_KEY_W) ==  GLFW_PRESS){
        light->transform.rotateGlobal(vec3(0.1,0.0,0));
    }

    
    if(glfwGetKey(this->gl_Window,GLFW_KEY_S) ==  GLFW_PRESS){
        light->transform.rotateGlobal(vec3(-0.1,0.0,0));

    }

    if(glfwGetKey(this->gl_Window,GLFW_KEY_D) ==  GLFW_PRESS){
        light->transform.rotateGlobal(vec3(0.0,0.1,0));
    }

    
    if(glfwGetKey(this->gl_Window,GLFW_KEY_A) ==  GLFW_PRESS){
        light->transform.rotateGlobal(vec3(0.0,-0.1,0));

    }
    
    



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
        float nearPlane = this->camera->getNearPlane();
        float farPlane = this->camera->getFarPlane();
        vec2 nearFar = vec2(nearPlane,farPlane);
        glBufferSubData(GL_UNIFORM_BUFFER, 32, sizeof(vec2), glm::value_ptr(nearFar));

        
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        //now the hard part with these buffers will be the lights, since we wil have 3 big vectors with stride things


        switch (type)
        {
        case ShaderType::Lit:
            shader = loadedShaders[type].get();
            shader->bindShader();
            setupShaderLighting(shader);
            //also set the skybox for enviroment reflection //Careful HERE, IT WIL BIND AUTOMATICALLY TO TEXT UNITY 0! -> WE
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, this->gl_SkyBox_Cubemap);
            loadedShaders[type]->setUniform("skybox",0);
            break;

        case ShaderType::Instanced:
            //printf("Hit!\n");
            shader = loadedShaders[type].get();
            shader->bindShader();
            setupShaderLighting(shader);
            //also set the skybox for enviroment reflection //Careful HERE, IT WIL BIND AUTOMATICALLY TO TEXT UNITY 0! -> WE
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, this->gl_SkyBox_Cubemap);
            shader->setUniform("time",(float)glfwGetTime());
            shader->setUniform("skybox",0);

            break;

        case ShaderType::Raymarch:
            shader = loadedShaders[type].get();
            loadedShaders[type]->bindShader();
            //this shader is lit so we set its lights
            setupShaderLighting(shader);
            shader->setUniform("skybox",0);
            break;
        
        default:
            break;
        }

        
    }

    //now on special shaders

        Shader* shader =  this->postProcessShader.get();
        shader->bindShader();
        setupShaderLighting(shader);



}

void Renderer::setupShaderLighting(Shader* shader){

    shader->setUniform("ambientLight",this->loadedScene->ambientLight);

    shader->setUniform("directionalLightCount", (int)this->loadedScene->getLights()[LightType::DIRECTIONAL].size());
    shader->setUniform("pointLightCount", (int)this->loadedScene->getLights()[LightType::POINT].size());
    shader->setUniform("spotLightCount", (int)this->loadedScene->getLights()[LightType::SPOT].size());

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


    //set shadow map array
    for(int i = 0;i<activeShadowCasters;i++){
        shader->setUniform("lightMats[" + std::to_string(i) +"]", this->gl_LightMatrices[i]);
    }
    glActiveTexture(GL_TEXTURE0+15);
    glBindTexture(GL_TEXTURE_2D_ARRAY,this->gl_ShadowMap_TEX_ARRAY);
    shader->setUniform("shadowMaps",15);
    shader->setUniform("activeShadowCasters",(int)this->activeShadowCasters);
    shader->setUniform("shadowMapSize",(float)this->gl_ShadowMap_Resolution);

}


void Renderer::shadowPass(){
    glViewport(0, 0, this->gl_ShadowMap_Resolution,this->gl_ShadowMap_Resolution);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    this->shadowMapShader->bindShader();
    
    for(int i = 0;i<this->activeShadowCasters;i++){
        glBindFramebuffer(GL_FRAMEBUFFER, this->gl_ShadowMap_FBOS[i]); //debugging
        glClear(GL_DEPTH_BUFFER_BIT);
       
       
       
       
        Light* light = this->shadowCasters.at(i).get();
       
       
        mat4 worldToLightTransform = light->getProjectionMatrix() * light->getViewMatrix();
        this->gl_LightMatrices[i] = worldToLightTransform;
        this->shadowMapShader->setUniform("lightSpaceMat",worldToLightTransform);
       
       
        //now, we just render everything, we do not consider transparency here, we dont care about any of that or their respective shaders
            const auto& models = this->loadedScene->getModels();
            for(auto modelIt = models.begin(); modelIt != models.end(); ++modelIt){
                Drawable* drawable = modelIt->get();
                if(drawable->getType() == DrawableType::MODEL){ //models cast shadows, usually
                    Model* model = static_cast<Model*>(drawable);
                    this->shadowMapShader->setUniform("modelMat",model->transform.getTransformMat());
                    model->draw(this->shadowMapShader.get());
               
                }
           
           
           
            }   
        }
    glCullFace(GL_BACK);
   
   
    //here we should have this light shadow map on the gl_ShadowMap_TEX
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    


}

void Renderer::geometryPass() {
    glViewport(0, 0, this->width, this->height);
    glBindFramebuffer(GL_FRAMEBUFFER, this->gl_Screen_FBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);  // ADD THIS LINE - reset draw buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
        //glCullFace(GL_BACK);


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
  
                    model->draw(shader);

                    //draw debug
                    debugShader->bindShader();
                    debugShader->setUniform("modelMat",model->transform.getTransformMat());
                    model->draw(debugShader);
                
                /*else{
                    //glStencilMask(0xFF);  // enable stencil writes
                    ////we will use the stencil buffer for some neat things
                    //glStencilFunc(GL_ALWAYS, 1, 0xFF);        // always pass
                    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // replace stencil with 1
                                                                     
                                                            // keep depth for original model
                    model->draw(shader);  
                                        debugShader->bindShader();
                    debugShader->setUniform("modelMat",model->transform.getTransformMat());
                    model->draw(debugShader);
                    //Log::write("[Renderer::geometryPass] - WARNING - OUTLINES ARE DISABLED UNTIL I SORT THEM CORRECTLY!");      
                                                        
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
                    */
                }

            else if(drawable->getType() == DrawableType::INSTANCED_MODEL){  
                glStencilMask(0x00);
                InstancedModel* model = static_cast<InstancedModel*>(drawable); 
                //loadedShaders[ShaderType::Instanced].get()->bindShader(); //not necessary
                //instanced models dont have uniform binding of the model matrix, this is solved by the model
                model->draw(shader);


            }


        }
    }



    
    //After that, we draw the skybox
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    skyboxShader->bindShader();
    skyboxShader->setUniform("projectionMat",camera->getProjectionMat());
    skyboxShader->setUniform("viewMat",glm::mat4(glm::mat3(camera->getViewMat())));
    glBindVertexArray(this->gl_Skybox_VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->gl_SkyBox_Cubemap);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    //is thereeaway to duplicated the texture so it does the depth testing but does not write to it
    //glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);  // Disable depth writes because of this, volumtric effects will render on top of the transparentt ones
    glEnable(GL_BLEND);
    for (auto& drawable : this->transparentDrawGroups) {
        Shader* shader = nullptr;
        if(drawable->getType() == DrawableType::MODEL){
            //bind the screen buffer
            glBindFramebuffer(GL_FRAMEBUFFER,this->gl_Screen_FBO);
            shader = loadedShaders[drawable->getShaderType()].get();
            Model* model = static_cast<Model*>(drawable.get()); 
            shader->bindShader();
            shader->setUniform("modelMat",model->transform.getTransformMat());
            shader->setUniform("normalMat",model->transform.getNormalMat());
            model->draw(shader);
        }
        else if(drawable->getType() == DrawableType::VOLUMETRIC){

            glBindFramebuffer(GL_READ_FRAMEBUFFER, this->gl_Screen_FBO);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->gl_Screen_Volumetric_FBO);
            glBlitFramebuffer(
                0, 0, width, height, 
                0, 0, width, height, 
                GL_DEPTH_BUFFER_BIT,  // Copy depth only
                GL_NEAREST
            );
            
            // Now render volumetric
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, this->gl_SkyBox_Cubemap);  // ← Add this!
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, this->gl_Screen_TEX);  
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, this->gl_Screen_DepthStencil_TEX);  
            
            Volumetric* volume = static_cast<Volumetric*>(drawable.get());
            volume->bindDensityField(3);
            shader = loadedShaders[drawable->getShaderType()].get();
            shader->bindShader();
            shader->setUniform("skybox", 0);           // ← Skybox on unit 0
            shader->setUniform("screenTexture", 1);
            shader->setUniform("screenDepth", 2);  
            shader->setUniform("time", (float)glfwGetTime());
            shader->setUniform("volumeCenter",volume->transform.getPosition());
            shader->setUniform("volumeDensity",3);
            shader->setUniform("volumeDimension",vec3(volume->width,volume->height,volume->length));
            shader->setUniform("scatteringCoefficient",volume->scatteringCoefficient);
        
            
            glBindVertexArray(this->gl_ScreenQuad_VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            // Copy result back (color only, preserve depth in Screen_FBO)
            glBindFramebuffer(GL_READ_FRAMEBUFFER, this->gl_Screen_Volumetric_FBO);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->gl_Screen_FBO);
            glBlitFramebuffer(
                0, 0, width, height, 
                0, 0, width, height, 
                GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, 
                GL_NEAREST
            );
    }
}
    //glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);



    glBindFramebuffer(GL_FRAMEBUFFER, 0); //0 is the default buffer and, per consequence, the windowe buffer
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->gl_Screen_TEX);  
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->gl_Screen_DepthStencil_TEX);  

    this->postProcessShader->bindShader();
    this->postProcessShader->setUniform("screenTexture", 0);
    this->postProcessShader->setUniform("screenDepth", 1);  
    this->postProcessShader->setUniform("time",(float) glfwGetTime());  

    glBindVertexArray(this->gl_ScreenQuad_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    




}



bool Renderer::isRunning() {
    return !glfwWindowShouldClose(this->gl_Window);
}

void Renderer::renderPass() {



        double frameStart = glfwGetTime();

        // Delta time
        this->currentFrame = static_cast<float>(frameStart);
        this->deltaTime = this->currentFrame - this->lastFrame;
        this->lastFrame = this->currentFrame;

        //std::cout << 1.0 / deltaTime << std::endl;

        processInput();

        shadowPass();
        geometryPass();

        glfwSwapBuffers(this->gl_Window);
        glfwPollEvents();

        // Frame time calculation
        double frameEnd = glfwGetTime();

    
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
    this->activeShadowCasters = scene->getLights()[LightType::DIRECTIONAL].size() + scene->getLights()[LightType::SPOT].size() ;//+ scene->getLights()[LightType::POINT].size() ;
    //adding casters
    const auto dirLights = scene->getLights()[LightType::DIRECTIONAL];
    for(auto it = dirLights.begin();it != dirLights.end();++it){
        this->shadowCasters.push_back(*it);
    }
    const auto spotLights = scene->getLights()[LightType::SPOT];
    for(auto it = spotLights.begin();it != spotLights.end();++it){
        this->shadowCasters.push_back(*it);
    }


    sortSceneModels();

    //update all sorting layers
}

void Renderer::framebuffer_size_callback(GLFWwindow* gl_Window, int width, int height) {
    glViewport(0, 0, width, height);
}