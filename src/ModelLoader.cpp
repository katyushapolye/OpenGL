#include "../headers/ModelLoader.h"





//for now, we dont deal with parented meshes!
Model* ModelLoader::loadFromObjWithAssimp(std::string path) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);
    
    // Check if scene loaded successfully - THIS IS CRITICAL
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        Log::write("[ModelLoader::loadFromObjWithAssimp] - ERROR: Failed to load file: " + path);
        Log::write("[ModelLoader::loadFromObjWithAssimp] - Error: " + std::string(importer.GetErrorString()));
        return nullptr;
    }
    
    Log::write("[ModelLoader::loadFromObjWithAssimp] - Successfully loaded: " + path + " with " + std::to_string(scene->mNumMeshes) + " meshes");
    
    std::vector<std::unique_ptr<Mesh>> meshes;
    
    // Process meshes
    for (int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* rawMesh = scene->mMeshes[i];
        Log::write("[ModelLoader::loadFromObjWithAssimp] - Processing mesh " + std::to_string(i) + " with " + std::to_string(rawMesh->mNumVertices) + " vertices");
        
        // Process vertices
        std::vector<Vertex> vertexes;
        for (int vCount = 0; vCount < rawMesh->mNumVertices; vCount++) {
            Vertex v;
            
            // Position (always present)
            v.position = vec3(rawMesh->mVertices[vCount].x,
                             rawMesh->mVertices[vCount].y,
                             rawMesh->mVertices[vCount].z);
            
            // Normal - CHECK IF EXISTS FIRST
            if (rawMesh->mNormals != nullptr) {
                v.normal = vec3(rawMesh->mNormals[vCount].x,
                               rawMesh->mNormals[vCount].y,
                               rawMesh->mNormals[vCount].z);
            } else {
                v.normal = vec3(0.0f, 1.0f, 0.0f); // Default up normal
            }
            
            // First texture coordinates - CHECK IF EXISTS
            if (rawMesh->mTextureCoords[0] != nullptr) {
                v.texCoords = vec2(rawMesh->mTextureCoords[0][vCount].x,
                                  rawMesh->mTextureCoords[0][vCount].y);
            } else {
                v.texCoords = vec2(0.0f, 0.0f);
            }
            
            // Second texture coordinates - CHECK IF EXISTS
            if (rawMesh->mTextureCoords[1] != nullptr) {
                v.texCoords2 = vec2(rawMesh->mTextureCoords[1][vCount].x,
                                   rawMesh->mTextureCoords[1][vCount].y);
            } else {
                v.texCoords2 = vec2(0.0f, 0.0f);
            }
            
            // Tangent and Bitangent - CHECK IF EXISTS
            if (rawMesh->mTangents != nullptr && rawMesh->mBitangents != nullptr) {
                v.tangent = vec3(rawMesh->mTangents[vCount].x,
                                rawMesh->mTangents[vCount].y,
                                rawMesh->mTangents[vCount].z);
                                
                v.bitangent = vec3(rawMesh->mBitangents[vCount].x,
                                  rawMesh->mBitangents[vCount].y,
                                  rawMesh->mBitangents[vCount].z);
            } else {
                v.tangent = vec3(1.0f, 0.0f, 0.0f);
                v.bitangent = vec3(0.0f, 0.0f, 1.0f);
            }
            
            vertexes.push_back(v);
        }
        
        // Process faces - YOU NEED THIS FOR INDICES
        std::vector<unsigned int> elements;
        for (int faceIdx = 0; faceIdx < rawMesh->mNumFaces; faceIdx++) {
            aiFace face = rawMesh->mFaces[faceIdx];
            for (int idx = 0; idx < face.mNumIndices; idx++) {
                elements.push_back(face.mIndices[idx]);
            }
        }
        
        // Adjust material index - subtract 1 to skip default material
        int adjustedMaterialIndex = rawMesh->mMaterialIndex - 1;
        if (adjustedMaterialIndex < 0) {
            Log::write("[ModelLoader::loadFromObjWithAssimp] - WARNING: Mesh " + std::to_string(i) + " using default material, setting to 0");
            adjustedMaterialIndex = 0;
        }
        
        std::string meshName = rawMesh->mName.C_Str();
        if (meshName.empty()) {
            meshName = "mesh_" + std::to_string(i);
        }
        
        Log::write("[ModelLoader::loadFromObjWithAssimp] - Created mesh '" + meshName + "' with " + 
                   std::to_string(vertexes.size()) + " vertices, " + std::to_string(elements.size()) + " indices, material: " + std::to_string(adjustedMaterialIndex));
        
        // Create mesh with adjusted material index
        try {
            meshes.push_back(std::unique_ptr<Mesh>(new Mesh(vertexes, elements, adjustedMaterialIndex)));
            Log::write("[ModelLoader::loadFromObjWithAssimp] - Successfully created mesh " + std::to_string(i));
        } catch (const std::exception& e) {
            Log::write("[ModelLoader::loadFromObjWithAssimp] - ERROR: Failed to create mesh " + std::to_string(i) + ": " + std::string(e.what()));
            return nullptr;
        }
    }
    
    // Process materials - Skip default material at index 0
    std::vector<std::unique_ptr<Material>> materials;
    Log::write("[ModelLoader::loadFromObjWithAssimp] - Processing " + std::to_string(scene->mNumMaterials - 1) + " materials (skipping default)");
    
    for (int i = 1; i < scene->mNumMaterials; i++) { // Start from 1 to skip default material
        aiMaterial* rawMat = scene->mMaterials[i];
        aiString diffusePath;
        aiString specularPath;
        aiString normalPath;
        if (rawMat->GetTexture(aiTextureType_DIFFUSE, 0, &diffusePath) == AI_SUCCESS) {
            float specularComponent;
            Log::write("[ModelLoader::loadFromObjWithAssimp] - Material " + std::to_string(i-1) + " texture path: " + std::string(diffusePath.C_Str()));

        } else {
            Log::write("[ModelLoader::loadFromObjWithAssimp] - Material " + std::to_string(i-1) + " has no diffuse texture");
        }
        if (rawMat->GetTexture(aiTextureType_SPECULAR, 0, &specularPath) == AI_SUCCESS) {
            Log::write("[ModelLoader::loadFromObjWithAssimp] - Material " + std::to_string(i-1) + " texture path: " + std::string(specularPath.C_Str()));

        } else {
            Log::write("[ModelLoader::loadFromObjWithAssimp] - Material " + std::to_string(i-1) + " has no specular texture");
        }
        if (rawMat->GetTexture(aiTextureType_NORMALS,0, &normalPath) == AI_SUCCESS) {

            Log::write("[ModelLoader::loadFromObjWithAssimp] - Material " + std::to_string(i-1) + " texture path: " + std::string(normalPath.C_Str()));

        } else {
            Log::write("[ModelLoader::loadFromObjWithAssimp] - Material " + std::to_string(i-1) + " has no normal texture");
        }

        //assemble the material
        float specularExponent;
        rawMat->Get(AI_MATKEY_SHININESS, specularExponent);
        float normalizedShininess = ((specularExponent / 1000.0f));
        materials.push_back(std::unique_ptr<Material>(new Material(TextureHandler::loadTexture(std::string(diffusePath.C_Str())).get(),
                                                           TextureHandler::loadTexture(std::string(specularPath.C_Str())).get(),
                                                           TextureHandler::loadTexture(std::string(normalPath.C_Str())).get(),
                                                           normalizedShininess, std::string(rawMat->GetName().C_Str())
    )));



    }
    
    Log::write("[ModelLoader::loadFromObjWithAssimp] - Model loading completed successfully");
    
    // TODO: Create and return actual Model object
    Log::write("[ModelLoader::loadFromObjWithAssimp] - About to return from function...");


    Model* model = new Model(std::move(meshes), std::move(materials));
    return model;
}

std::vector<Material*> ModelLoader::loadMTLMaterial(std::string path){
    std::vector<Material*> mats;
    std::ifstream inputFile;
    inputFile.open(path);
    
    if(!inputFile.is_open()){
        Log::write("[ModelLoader::loadMTLMaterial]-Failed to open file of name: " + path);
        return mats;
    }
    
    std::string line;
    std::string prefix;
    float auxFloat;
    int auxInt;
    std::string auxString;
    float x, y, z;
    
    // Current material properties
    std::string name;
    glm::vec3 diffuseColor(0.8f, 0.8f, 0.8f);   // Default diffuse
    glm::vec3 specularColor(0.0f, 0.0f, 0.0f);  // Default specular
    float specularComponent = 0.0f;
    float dissolve = 1.0f;
    int illuminationModel = 1;
    std::string diffusePath;
    std::string specularPath;
    std::string normalPath;
    
    // Track if we have a current material being processed
    bool hasMaterial = false;
    
    while(std::getline(inputFile, line)){
        // Skip empty lines and comments
        if(line.empty() || line[0] == '#') {
            continue;
        }
        
        std::istringstream iss(line);
        iss >> prefix;
        
        if(prefix == "newmtl"){
            // If we already have a material, save it before starting a new one
            if(hasMaterial){
                // Create material with current properties
                Texture* diffuse = nullptr;
                Texture* specular = nullptr;
                Texture* normal = nullptr;
                
                if(!diffusePath.empty()){
                    diffuse = (TextureHandler::loadTexture("Textures/"+diffusePath)).get();

                }
                if(!specularPath.empty()){
                    specular = (TextureHandler::loadTexture("Textures/"+specularPath)).get();
                }
                if(!normalPath.empty()){
                    normal = (TextureHandler::loadTexture("Textures/"+normalPath)).get();
                }
                
                mats.push_back(new Material(diffuse, specular, normal, specularComponent, name));
            }
            
            // Start new material
            iss >> name;
            hasMaterial = true;
            
            // Reset properties to defaults
            diffuseColor = glm::vec3(0.8f, 0.8f, 0.8f);
            specularColor = glm::vec3(0.0f, 0.0f, 0.0f);
            specularComponent = 0.0f;
            dissolve = 1.0f;
            illuminationModel = 1;
            diffusePath.clear();
            specularPath.clear();
            normalPath.clear();
        }
        else if(prefix == "Kd" && hasMaterial){
            iss >> x >> y >> z;
            diffuseColor = glm::vec3(x, y, z);
        }
        else if(prefix == "Ks" && hasMaterial){
            iss >> x >> y >> z;
            specularColor = glm::vec3(x, y, z);
        }
        else if(prefix == "Ns" && hasMaterial){
            iss >> auxFloat;
            specularComponent = auxFloat;
        }
        else if(prefix == "d" && hasMaterial){
            iss >> auxFloat;
            dissolve = auxFloat;
        }
        else if(prefix == "illum" && hasMaterial){  // Fixed typo: was "ilum"
            iss >> auxInt;
            illuminationModel = auxInt;
        }
        else if(prefix == "map_Kd" && hasMaterial){
            iss >> auxString;
            diffusePath = auxString;
        }
        else if(prefix == "map_Ks" && hasMaterial){
            iss >> auxString;
            specularPath = auxString;
        }
        else if(prefix == "map_bump" && hasMaterial){
            iss >> auxString;
            normalPath = auxString;
        }
    }
    
    // Don't forget the last material if file doesn't end with a new material
    if(hasMaterial){
        Texture* diffuse = nullptr;
        Texture* specular = nullptr;
        Texture* normal = nullptr;
        
        if(!diffusePath.empty()){
                diffuse = (TextureHandler::loadTexture("Textures/"+diffusePath)).get();
        }
        if(!specularPath.empty()){
            specular =  (TextureHandler::loadTexture("Textures/"+specularPath)).get();
        }
        if(!normalPath.empty()){
            normal = (TextureHandler::loadTexture("Textures/"+normalPath)).get();
        }

        
        mats.push_back(new Material(diffuse, specular, normal, specularComponent, name));


    }
    
    inputFile.close();
    return mats;
}