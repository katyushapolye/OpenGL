#include "../headers/Mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indexes, unsigned int materialIndex) {
    this->gl_MaterialIndex = materialIndex;
    this->gl_Vertices = vertices;
    this->gl_Indices = indexes;
    this->elementCount = indexes.size();
    
    initializeMesh(vertices, indexes);
}

void Mesh::initializeMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indexes) {
    // Generate and bind VAO, VBO, EBO
    glGenVertexArrays(1, &gl_VAO);  
    glGenBuffers(1, &gl_VBO);      
    glGenBuffers(1, &gl_EBO);
    glBindVertexArray(gl_VAO);
    
    // Create interleaved vertex data
    std::vector<float> vertexData;
    vertexData.reserve(vertices.size() * 16); // 16 floats per vertex
    
    for (const auto& vertex : vertices) {
        // Position (3 floats)
        vertexData.insert(vertexData.end(), {vertex.position.x, vertex.position.y, vertex.position.z});
        // Normal (3 floats)
        vertexData.insert(vertexData.end(), {vertex.normal.x, vertex.normal.y, vertex.normal.z});
        // Tangent (3 floats)
        vertexData.insert(vertexData.end(), {vertex.tangent.x, vertex.tangent.y, vertex.tangent.z});
        // Bitangent (3 floats)
        vertexData.insert(vertexData.end(), {vertex.bitangent.x, vertex.bitangent.y, vertex.bitangent.z});
        // Primary texture coordinates (2 floats)
        vertexData.insert(vertexData.end(), {vertex.texCoords.s, vertex.texCoords.t});
        // Secondary texture coordinates (2 floats)
        vertexData.insert(vertexData.end(), {vertex.texCoords2.s, vertex.texCoords2.t});
    }
    
    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, gl_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
    
    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * sizeof(unsigned int), indexes.data(), GL_STATIC_DRAW);
    
    // Set up vertex attributes
    const size_t stride = 16 * sizeof(float);
    
    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Tangent attribute (location 2)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // Bitangent attribute (location 3)
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    // Primary texture coordinates (location 4)
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, (void*)(12 * sizeof(float)));
    glEnableVertexAttribArray(4);
    
    // Secondary texture coordinates (location 5)
    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, stride, (void*)(14 * sizeof(float)));
    glEnableVertexAttribArray(5);
}

unsigned int Mesh::getMaterialIndex() {
    return gl_MaterialIndex;
}

void Mesh::draw(Shader* shader, Material* mat) {
    shader->bindShader();
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
    
    glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0);
}