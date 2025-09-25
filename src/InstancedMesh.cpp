#include "../headers/InstancedMesh.h"

InstancedMesh::InstancedMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indexes, unsigned int materialIndex) {
    this->gl_MaterialIndex = materialIndex;
    this->gl_Vertices = vertices;
    this->gl_Indices = indexes;
    this->elementCount = indexes.size();
    
    initializeInstancedMesh(vertices, indexes);
}

void InstancedMesh::initializeInstancedMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indexes) {
    // Generate all buffers
    glGenVertexArrays(1, &gl_VAO);  
    glGenBuffers(1, &gl_VBO);
    glGenBuffers(1, &gl_Instance_VBO);  
    glGenBuffers(1, &gl_EBO);
    
    glBindVertexArray(gl_VAO);
    
    // Create interleaved vertex data (position, normal, texCoords = 8 floats per vertex)
    std::vector<float> vertexData;
    vertexData.reserve(vertices.size() * 8);
    
    for (const auto& vertex : vertices) {
        // Position (3 floats)
        vertexData.insert(vertexData.end(), {vertex.position.x, vertex.position.y, vertex.position.z});
        // Normal (3 floats)
        vertexData.insert(vertexData.end(), {vertex.normal.x, vertex.normal.y, vertex.normal.z});
        // Primary texture coordinates (2 floats)
        vertexData.insert(vertexData.end(), {vertex.texCoords.s, vertex.texCoords.t});
    }
    

    glBindBuffer(GL_ARRAY_BUFFER, gl_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
    

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * sizeof(unsigned int), indexes.data(), GL_STATIC_DRAW);
    

    const size_t stride = 8 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);                    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float))); // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float))); // TexCoords
    glEnableVertexAttribArray(2);

    // 
    // See, i didnt know that but the VAO knows both the data layout AND from what buffer it should read from
    glBindBuffer(GL_ARRAY_BUFFER, gl_Instance_VBO);
    
    // Matrix is 4x4, so we need 4 vec4 attributes
    for (int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), 
                             (void*)(i * sizeof(glm::vec4)));

        glVertexAttribDivisor(3 + i, 1); // Tell OpenGL this is a per-instance attribute
    }
    
    // Unbind VAO
    glBindVertexArray(0);
}

unsigned int InstancedMesh::getMaterialIndex() {
    return gl_MaterialIndex;
}

void InstancedMesh::InstancedDraw(Shader* shader, Material* mat, std::vector<mat4>& instanceMatrices) {
    shader->bindShader();
    
    // Update instance buffer with new matrices
    glBindBuffer(GL_ARRAY_BUFFER, gl_Instance_VBO);
    glBufferData(GL_ARRAY_BUFFER, instanceMatrices.size() * sizeof(glm::mat4), 
                 instanceMatrices.data(), GL_DYNAMIC_DRAW);
    
    glBindVertexArray(gl_VAO);
    
    // Set material uniforms
    const std::string materialBase = "material0.";
    shader->setUniform(materialBase + "diffuseColor", mat->diffuseColor);
    shader->setUniform(materialBase + "shininess", mat->shininess);
    
    // Bind textures
    int textureUnit = 1;
    
    mat->diffuseMap->bindTexture(textureUnit);
    shader->setUniform(materialBase + "diffuseMap", textureUnit++);
    
    mat->specularMap->bindTexture(textureUnit);
    shader->setUniform(materialBase + "specularMap", textureUnit++);
    
    mat->reflectionMap->bindTexture(textureUnit);
    shader->setUniform(materialBase + "reflectionMap", textureUnit++);
    
    mat->normalMap->bindTexture(textureUnit);
    shader->setUniform(materialBase + "normalMap", textureUnit);
    
    // Draw instanced geometry
    glDrawElementsInstanced(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0, instanceMatrices.size());
}