#include "../headers/ModelLoader.h"

Model* ModelLoader::loadFromObj(std::string path){
    std::ifstream inputFile;
    std::vector<Material*> allMats;
    std::map<std::string, int> materialNameToIndex;
    
    inputFile.open(path);
    std::vector<vec3> vertexPositions;
    std::vector<vec3> vertexNormals;
    std::vector<vec2> vertexUV;
    
    // Structure to hold mesh data
    struct MeshData {
        std::string name;
        std::vector<int> indexPos;
        std::vector<int> indexNormal;
        std::vector<int> indexUV;
        std::string materialName;
        int materialIndex = -1;
    };
    
    std::vector<MeshData> meshes;
    MeshData* currentMesh = nullptr;
    
    std::string line;
    float x,y,z;
    
    if(inputFile.is_open()){

        while(std::getline(inputFile, line)){
            // Skip comments and empty lines
            if(line.empty() || line[0] == '#'){
                continue;
            }
            
            std::istringstream iss(line);
            std::string prefix;
            std::string aux;
            iss >> prefix;

            if(prefix ==  "mtllib"){
                iss >> aux;
                std::vector<Material*> newMats = loadMTLMaterial("Materials/" + aux);
                
                // Add new materials to the collection and update name-to-index mapping
                for(Material* mat : newMats) {
                    std::string matName = mat->name; // Assuming Material has getName() method
                    if(materialNameToIndex.find(matName) == materialNameToIndex.end()) {
                        materialNameToIndex[matName] = allMats.size();
                        allMats.push_back(mat);
                        Log::write("[ModelLoader::loadFromObj] - Loaded material: " + matName + " at index " + std::to_string(allMats.size() - 1));
                    }
                }
            }
            else if(prefix == "o"){
                // New object/mesh
                iss >> aux;
                meshes.push_back(MeshData());
                currentMesh = &meshes.back();
                currentMesh->name = aux;
                Log::write("[ModelLoader::loadFromObj] - Starting new mesh: " + aux);
            }
            else if(prefix == "usemtl"){
                // Material assignment for current mesh
                if(currentMesh != nullptr){
                    iss >> aux;
                    currentMesh->materialName = aux;
                    // Find material index using the map
                    auto it = materialNameToIndex.find(aux);
                    if(it != materialNameToIndex.end()) {
                        currentMesh->materialIndex = it->second;
                        Log::write("[ModelLoader::loadFromObj] - Assigned material '" + aux + "' (index " + std::to_string(it->second) + ") to mesh '" + currentMesh->name + "'");
                    } else {
                        Log::write("[ModelLoader::loadFromObj] - WARNING: Material '" + aux + "' not found for mesh '" + currentMesh->name + "'");
                    }
                }
            }
            else if(prefix == "v"){
                iss >> x >> y >> z;
                vertexPositions.push_back(vec3(x,y,z));
            }
            else if(prefix == "vt"){
                iss >> x >> y;
                vertexUV.push_back(vec2(x,y));
            }
            else if(prefix == "vn"){
                iss >> x >> y >> z;
                vertexNormals.push_back(vec3(x,y,z));
            }
            else if(prefix == "f"){
                // If no mesh has been defined yet, create a default one
                if(currentMesh == nullptr){
                    meshes.push_back(MeshData());
                    currentMesh = &meshes.back();
                    currentMesh->name = "default";
                }
                
                std::string element1, element2, element3;
                iss >> element1 >> element2 >> element3;
                
                // Parse element1 (vertex 1)
                std::stringstream ss1(element1);
                std::string token;
                int pos1 = -1, uv1 = -1, normal1 = -1;
                int index = 0;
                while(std::getline(ss1, token, '/')) {
                    if(!token.empty()) {
                        if(index == 0) pos1 = std::stoi(token) - 1;      // position index
                        else if(index == 1) uv1 = std::stoi(token) - 1;  // UV index
                        else if(index == 2) normal1 = std::stoi(token) - 1; // normal index
                    }
                    index++;
                }
                
                // Parse element2 (vertex 2)
                std::stringstream ss2(element2);
                int pos2 = -1, uv2 = -1, normal2 = -1;
                index = 0;
                while(std::getline(ss2, token, '/')) {
                    if(!token.empty()) {
                        if(index == 0) pos2 = std::stoi(token) - 1;
                        else if(index == 1) uv2 = std::stoi(token) - 1;
                        else if(index == 2) normal2 = std::stoi(token) - 1;
                    }
                    index++;
                }
                
                // Parse element3 (vertex 3)
                std::stringstream ss3(element3);
                int pos3 = -1, uv3 = -1, normal3 = -1;
                index = 0;
                while(std::getline(ss3, token, '/')) {
                    if(!token.empty()) {
                        if(index == 0) pos3 = std::stoi(token) - 1;
                        else if(index == 1) uv3 = std::stoi(token) - 1;
                        else if(index == 2) normal3 = std::stoi(token) - 1;
                    }
                    index++;
                }
                
                // Validate indices before storing
                if(pos1 >= 0 && pos1 < vertexPositions.size() &&
                   pos2 >= 0 && pos2 < vertexPositions.size() &&
                   pos3 >= 0 && pos3 < vertexPositions.size()) {
                    
                    currentMesh->indexPos.push_back(pos1);
                    currentMesh->indexPos.push_back(pos2);
                    currentMesh->indexPos.push_back(pos3);
                    
                    // Handle UV indices (may not exist)
                    currentMesh->indexUV.push_back(uv1 >= 0 && uv1 < vertexUV.size() ? uv1 : 0);
                    currentMesh->indexUV.push_back(uv2 >= 0 && uv2 < vertexUV.size() ? uv2 : 0);
                    currentMesh->indexUV.push_back(uv3 >= 0 && uv3 < vertexUV.size() ? uv3 : 0);
                    
                    // Handle normal indices (may not exist)
                    currentMesh->indexNormal.push_back(normal1 >= 0 && normal1 < vertexNormals.size() ? normal1 : 0);
                    currentMesh->indexNormal.push_back(normal2 >= 0 && normal2 < vertexNormals.size() ? normal2 : 0);
                    currentMesh->indexNormal.push_back(normal3 >= 0 && normal3 < vertexNormals.size() ? normal3 : 0);
                }
            }
        }
        inputFile.close();
        
        // Debug output
        Log::write("[ModelLoader::loadFromObj] - Loaded Model with:");
        Log::write("[ModelLoader::loadFromObj] -- Positions: " + std::to_string(vertexPositions.size()));
        Log::write("[ModelLoader::loadFromObj] -- Normals: " + std::to_string(vertexNormals.size()));
        Log::write("[ModelLoader::loadFromObj] -- UVs: " + std::to_string(vertexUV.size()));
        Log::write("[ModelLoader::loadFromObj] -- Meshes: " + std::to_string(meshes.size()));
    }
    else{
        Log::write("[ModelLoader::.loadFromObj] - Failed to open file of name:" + path);
        return nullptr;
    }
    
    // Check if we have any data
    if(meshes.empty()) {
        Log::write("[ModelLoader::loadFromObj] - No Valid GeometryData Found");
        return nullptr;
    }
    
    // Create GL meshes from mesh data
    std::vector<std::unique_ptr<Mesh>> gl_Meshes;
    
    for(const auto& meshData : meshes){
        if(meshData.indexPos.empty()){
            Log::write("[ModelLoader::loadFromObj] - Skipping empty mesh: " + meshData.name);
            continue;
        }
        
        std::vector<Vertex> gl_Vertices;
        std::vector<unsigned int> gl_Elements;
        
        // Create vertices from the face data
        for(size_t i = 0; i < meshData.indexPos.size(); i++){
            // Debug output for problematic indices
            if(meshData.indexPos[i] >= vertexPositions.size()) {
                Log::write("[ModelLoader::loadFromObj] - ERROR: Position index " + std::to_string(meshData.indexPos[i]) + " >= " + std::to_string(vertexPositions.size()) + " in mesh " + meshData.name);
                continue;
            }
            
            Vertex v;
            v.position = vertexPositions[meshData.indexPos[i]];
            
            // Handle normals
            if(i < meshData.indexNormal.size() && meshData.indexNormal[i] >= 0 && meshData.indexNormal[i] < vertexNormals.size()) {
                v.normal = vertexNormals[meshData.indexNormal[i]];
            } else {
                v.normal = vec3(0.0f, 0.0f, 1.0f); // Default normal
            }
            
            // Handle UVs  
            if(i < meshData.indexUV.size() && meshData.indexUV[i] >= 0 && meshData.indexUV[i] < vertexUV.size()) {
                v.texCoords = vertexUV[meshData.indexUV[i]];
            } else {
                v.texCoords = vec2(0.0f, 0.0f); // Default UV
            }
            
            gl_Vertices.push_back(v);
            gl_Elements.push_back(i); // Sequential element indices
        }
        
        // Create the mesh
        std::unique_ptr<Mesh> mesh = std::unique_ptr<Mesh>(new Mesh(gl_Vertices, gl_Elements,0));
        
        // Set material index if available
        //if(meshData.materialIndex >= 0) {
        //    // Assuming Mesh class has a setMaterialIndex method
        //    // Adjust this based on your Mesh class implementation
        //    mesh->setMaterialIndex(meshData.materialIndex);
        //}
        
        gl_Meshes.push_back(std::move(mesh));
        
        Log::write("[ModelLoader::loadFromObj] - Created mesh '" + meshData.name + "' with " + 
                   std::to_string(gl_Vertices.size()) + " vertices, material: " + meshData.materialName);
    }
    
    // Create materials vector
    std::vector<std::unique_ptr<Material>> materials;
    for(int i = 0; i < allMats.size(); i++){
        materials.push_back(std::unique_ptr<Material>(allMats[i]));
    }

    Model* model = new Model(std::move(gl_Meshes), std::move(materials));
    return model;
}



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