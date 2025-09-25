#include "../headers/ModelLoader.h"






Model* ModelLoader::loadFromObj(std::string path) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        Log::write("[ModelLoader::loadFromObjWithAssimp] - ERROR: Failed to load file: " + path);
        Log::write("[ModelLoader::loadFromObjWithAssimp] - Error: " + std::string(importer.GetErrorString()));
        return nullptr;
    }

    Log::write("[ModelLoader::loadFromObjWithAssimp] - Successfully loaded: " + path + " with " + std::to_string(scene->mNumMeshes) + " meshes");

    std::vector<std::unique_ptr<Mesh>> meshes;

    // ----------- MESH PROCESSING (unchanged) -----------
    for (int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* rawMesh = scene->mMeshes[i];

        std::vector<Vertex> vertexes;
        for (int vCount = 0; vCount < rawMesh->mNumVertices; vCount++) {
            Vertex v;
            v.position = vec3(rawMesh->mVertices[vCount].x,
                              rawMesh->mVertices[vCount].y,
                              rawMesh->mVertices[vCount].z);

            if (rawMesh->mNormals) {
                v.normal = vec3(rawMesh->mNormals[vCount].x,
                                rawMesh->mNormals[vCount].y,
                                rawMesh->mNormals[vCount].z);
            } else {
                v.normal = vec3(0.0f, 1.0f, 0.0f);
            }

            if (rawMesh->mTextureCoords[0]) {
                v.texCoords = vec2(rawMesh->mTextureCoords[0][vCount].x,
                                   rawMesh->mTextureCoords[0][vCount].y);
            } else {
                v.texCoords = vec2(0.0f, 0.0f);
            }

            if (rawMesh->mTextureCoords[1]) {
                v.texCoords2 = vec2(rawMesh->mTextureCoords[1][vCount].x,
                                    rawMesh->mTextureCoords[1][vCount].y);
            } else {
                v.texCoords2 = vec2(0.0f, 0.0f);
            }

            if (rawMesh->mTangents && rawMesh->mBitangents) {
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

        std::vector<unsigned int> elements;
        for (int faceIdx = 0; faceIdx < rawMesh->mNumFaces; faceIdx++) {
            aiFace face = rawMesh->mFaces[faceIdx];
            for (int idx = 0; idx < face.mNumIndices; idx++) {
                elements.push_back(face.mIndices[idx]);
            }
        }

        int adjustedMaterialIndex = rawMesh->mMaterialIndex - 1;
        if (adjustedMaterialIndex < 0) {
            Log::write("[ModelLoader::loadFromObjWithAssimp] - WARNING: Mesh " + std::to_string(i) + " using default material, setting to 0");
            adjustedMaterialIndex = 0;
        }

        meshes.push_back(std::unique_ptr<Mesh>(new Mesh(vertexes, elements, adjustedMaterialIndex)));
    }

    // ----------- MATERIAL PROCESSING -----------
    std::vector<std::unique_ptr<Material>> materials;
    Log::write("[ModelLoader::loadFromObjWithAssimp] - Processing " + std::to_string(scene->mNumMaterials - 1) + " materials (skipping default)");

    for (int i = 1; i < scene->mNumMaterials; i++) {
        aiMaterial* rawMat = scene->mMaterials[i];
        aiString diffusePath, specularPath, normalPath, reflectionPath;

        bool hasDiffuse   = rawMat->GetTexture(aiTextureType_DIFFUSE,   0, &diffusePath)   == AI_SUCCESS;
        bool hasSpecular  = rawMat->GetTexture(aiTextureType_SPECULAR,  0, &specularPath)  == AI_SUCCESS;
        bool hasNormal    = rawMat->GetTexture(aiTextureType_NORMALS,   0, &normalPath)    == AI_SUCCESS;
        bool hasReflect   = rawMat->GetTexture(aiTextureType_AMBIENT,   0, &reflectionPath)== AI_SUCCESS;

        aiColor3D diffuseColor(1.f, 0.f, 1.f); // magenta default
        rawMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
        vec3 difCol = vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b);

        float specularExponent = 0.0f;
        rawMat->Get(AI_MATKEY_SHININESS, specularExponent);
        float normalizedShininess = (specularExponent / 1000.0f);

        try {
            if (!hasDiffuse && !hasSpecular && !hasNormal && !hasReflect) {
                Log::write("[ModelLoader::loadFromObjWithAssimp] - ERROR: Material " + std::to_string(i-1) + " is incomplete, using MAGENTA fallback");

                materials.push_back(std::unique_ptr<Material>(new Material(
                    TextureHandler::loadTexture("DIFFUSE_FALLBACK",TextureType::DIFFUSE).get(), // empty texture let the handler solve it
                    TextureHandler::loadTexture("",TextureType::SPECULAR).get(),
                    TextureHandler::loadTexture("",TextureType::NORMAL).get(),
                    TextureHandler::loadTexture("",TextureType::REFLECTION).get(),
                    vec3(1.0f, 0.0f, 1.0f), // magenta
                    1.0f,
                    "FallbackMagenta"
                )));
            } else {
                materials.push_back(std::unique_ptr<Material>(new Material(
                    TextureHandler::loadTexture(hasDiffuse  ? diffusePath.C_Str()   : "DIFFUSE_FALLBACK",TextureType::DIFFUSE).get(),
                    TextureHandler::loadTexture(hasSpecular ? specularPath.C_Str()  : "",TextureType::SPECULAR).get(),
                    TextureHandler::loadTexture(hasNormal   ? normalPath.C_Str()    : "",TextureType::NORMAL).get(),
                    TextureHandler::loadTexture(hasReflect  ? reflectionPath.C_Str(): "",TextureType::REFLECTION).get(),
                    difCol,
                    normalizedShininess,
                    std::string(rawMat->GetName().C_Str())
                )));
            }
        } catch (const std::exception& e) {
            Log::write("[ModelLoader::loadFromObjWithAssimp] - ERROR: Failed to load material " + std::to_string(i-1) + ": " + std::string(e.what()) + " — using MAGENTA fallback");
            materials.push_back(std::unique_ptr<Material>(new Material(
                TextureHandler::loadTexture("DIFUSSE_FALLBACK",TextureType::DIFFUSE).get(),
                TextureHandler::loadTexture("",TextureType::SPECULAR).get(),
                TextureHandler::loadTexture("",TextureType::NORMAL).get(),
                TextureHandler::loadTexture("",TextureType::REFLECTION).get(),
                vec3(1.0f, 0.0f, 1.0f),
                1.0f,
                "FallbackMagenta"
            )));
        }
    }

    Log::write("[ModelLoader::loadFromObjWithAssimp] - Model loading completed successfully");
    Model* model = new Model(std::move(meshes), std::move(materials));
    return model;
}



InstancedModel* ModelLoader::loadFromObjAsInstanced(std::string path){
        Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        Log::write("[ModelLoader::loadFromObjWithAssimp] - ERROR: Failed to load file: " + path);
        Log::write("[ModelLoader::loadFromObjWithAssimp] - Error: " + std::string(importer.GetErrorString()));
        return nullptr;
    }

    Log::write("[ModelLoader::loadFromObjWithAssimp] - Successfully loaded: " + path + " with " + std::to_string(scene->mNumMeshes) + " meshes");

    std::vector<std::unique_ptr<InstancedMesh>> meshes;

    // ----------- MESH PROCESSING (unchanged) -----------
    for (int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* rawMesh = scene->mMeshes[i];

        std::vector<Vertex> vertexes;
        for (int vCount = 0; vCount < rawMesh->mNumVertices; vCount++) {
            Vertex v;
            v.position = vec3(rawMesh->mVertices[vCount].x,
                              rawMesh->mVertices[vCount].y,
                              rawMesh->mVertices[vCount].z);

            if (rawMesh->mNormals) {
                v.normal = vec3(rawMesh->mNormals[vCount].x,
                                rawMesh->mNormals[vCount].y,
                                rawMesh->mNormals[vCount].z);
            } else {
                v.normal = vec3(0.0f, 1.0f, 0.0f);
            }

            if (rawMesh->mTextureCoords[0]) {
                v.texCoords = vec2(rawMesh->mTextureCoords[0][vCount].x,
                                   rawMesh->mTextureCoords[0][vCount].y);
            } else {
                v.texCoords = vec2(0.0f, 0.0f);
            }

            if (rawMesh->mTextureCoords[1]) {
                v.texCoords2 = vec2(rawMesh->mTextureCoords[1][vCount].x,
                                    rawMesh->mTextureCoords[1][vCount].y);
            } else {
                v.texCoords2 = vec2(0.0f, 0.0f);
            }

            if (rawMesh->mTangents && rawMesh->mBitangents) {
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

        std::vector<unsigned int> elements;
        for (int faceIdx = 0; faceIdx < rawMesh->mNumFaces; faceIdx++) {
            aiFace face = rawMesh->mFaces[faceIdx];
            for (int idx = 0; idx < face.mNumIndices; idx++) {
                elements.push_back(face.mIndices[idx]);
            }
        }

        int adjustedMaterialIndex = rawMesh->mMaterialIndex - 1;
        if (adjustedMaterialIndex < 0) {
            Log::write("[ModelLoader::loadFromObjWithAssimp] - WARNING: Mesh " + std::to_string(i) + " using default material, setting to 0");
            adjustedMaterialIndex = 0;
        }

        meshes.push_back(std::unique_ptr<InstancedMesh>(new InstancedMesh(vertexes, elements, adjustedMaterialIndex)));
    }

    // ----------- MATERIAL PROCESSING -----------
    std::vector<std::unique_ptr<Material>> materials;
    Log::write("[ModelLoader::loadFromObjWithAssimp] - Processing " + std::to_string(scene->mNumMaterials - 1) + " materials (skipping default)");

    for (int i = 1; i < scene->mNumMaterials; i++) {
        aiMaterial* rawMat = scene->mMaterials[i];
        aiString diffusePath, specularPath, normalPath, reflectionPath;

        bool hasDiffuse   = rawMat->GetTexture(aiTextureType_DIFFUSE,   0, &diffusePath)   == AI_SUCCESS;
        bool hasSpecular  = rawMat->GetTexture(aiTextureType_SPECULAR,  0, &specularPath)  == AI_SUCCESS;
        bool hasNormal    = rawMat->GetTexture(aiTextureType_NORMALS,   0, &normalPath)    == AI_SUCCESS;
        bool hasReflect   = rawMat->GetTexture(aiTextureType_AMBIENT,   0, &reflectionPath)== AI_SUCCESS;

        aiColor3D diffuseColor(1.f, 0.f, 1.f); // magenta default
        rawMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
        vec3 difCol = vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b);

        float specularExponent = 0.0f;
        rawMat->Get(AI_MATKEY_SHININESS, specularExponent);
        float normalizedShininess = (specularExponent / 1000.0f);

        try {
            if (!hasDiffuse && !hasSpecular && !hasNormal && !hasReflect) {
                Log::write("[ModelLoader::loadFromObjWithAssimp] - ERROR: Material " + std::to_string(i-1) + " is incomplete, using MAGENTA fallback");

                materials.push_back(std::unique_ptr<Material>(new Material(
                    TextureHandler::loadTexture("DIFFUSE_FALLBACK",TextureType::DIFFUSE).get(), // empty texture let the handler solve it
                    TextureHandler::loadTexture("",TextureType::SPECULAR).get(),
                    TextureHandler::loadTexture("",TextureType::NORMAL).get(),
                    TextureHandler::loadTexture("",TextureType::REFLECTION).get(),
                    vec3(1.0f, 0.0f, 1.0f), // magenta
                    1.0f,
                    "FallbackMagenta"
                )));
            } else {
                materials.push_back(std::unique_ptr<Material>(new Material(
                    TextureHandler::loadTexture(hasDiffuse  ? diffusePath.C_Str()   : "DIFFUSE_FALLBACK",TextureType::DIFFUSE).get(),
                    TextureHandler::loadTexture(hasSpecular ? specularPath.C_Str()  : "",TextureType::SPECULAR).get(),
                    TextureHandler::loadTexture(hasNormal   ? normalPath.C_Str()    : "",TextureType::NORMAL).get(),
                    TextureHandler::loadTexture(hasReflect  ? reflectionPath.C_Str(): "",TextureType::REFLECTION).get(),
                    difCol,
                    normalizedShininess,
                    std::string(rawMat->GetName().C_Str())
                )));
            }
        } catch (const std::exception& e) {
            Log::write("[ModelLoader::loadFromObjWithAssimp] - ERROR: Failed to load material " + std::to_string(i-1) + ": " + std::string(e.what()) + " — using MAGENTA fallback");
            materials.push_back(std::unique_ptr<Material>(new Material(
                TextureHandler::loadTexture("DIFUSSE_FALLBACK",TextureType::DIFFUSE).get(),
                TextureHandler::loadTexture("",TextureType::SPECULAR).get(),
                TextureHandler::loadTexture("",TextureType::NORMAL).get(),
                TextureHandler::loadTexture("",TextureType::REFLECTION).get(),
                vec3(1.0f, 0.0f, 1.0f),
                1.0f,
                "FallbackMagenta"
            )));
        }
    }

    Log::write("[ModelLoader::loadFromObjWithAssimp] - Model loading completed successfully");
    InstancedModel* model = new InstancedModel(std::move(meshes), std::move(materials));
    return model;

}

