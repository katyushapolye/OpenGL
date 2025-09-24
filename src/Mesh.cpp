#include "../headers/Mesh.h"




//we need to update intilize
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indexes, unsigned int materialIndex) {
    std::vector<vec3> verts;
    std::vector<vec3> normals;
    std::vector<vec3> tangents;
    std::vector<vec3> bitangents;
    std::vector<vec2> texCoords;
    std::vector<vec2> texCoords2;
    this->gl_MaterialIndex = materialIndex;
   
    // Extract all attributes from the vertex data
    for (const auto& vertex : vertices) {
        verts.push_back(vertex.position);
        normals.push_back(vertex.normal);
        tangents.push_back(vertex.tangent);
        bitangents.push_back(vertex.bitangent);
        texCoords.push_back(vertex.texCoords);
        texCoords2.push_back(vertex.texCoords2);
    }
   
    initializeMesh(verts, normals, tangents, bitangents, texCoords, texCoords2, indexes);
    this->gl_Vertices = vertices;
    this->gl_Indices = indexes;
}

// Updated initializeMesh function signature and implementation
    void Mesh::initializeMesh(std::vector<vec3>& verts, 
                         std::vector<vec3>& normals, 
                         std::vector<vec3>& tangents,
                         std::vector<vec3>& bitangents,
                         std::vector<vec2>& texCoords,
                         std::vector<vec2>& texCoords2,
                         std::vector<unsigned int>& indexes) {
    this->gl_VBO = 0;
    this->gl_VAO = 0;
    this->gl_EBO = 0;
   
    glGenVertexArrays(1, &this->gl_VAO);  
    glGenBuffers(1, &this->gl_VBO);      
    glGenBuffers(1, &this->gl_EBO);
    glBindVertexArray(this->gl_VAO);
   
    setVertexArray(verts, normals, tangents, bitangents, texCoords, texCoords2);
    setIndexArray(indexes);
   
    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
   
    // Normal attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
   
    // Tangent attribute (location 2)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
   
    // Bitangent attribute (location 3)
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);
   
    // Primary texture coordinates (location 4)
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(12 * sizeof(float)));
    glEnableVertexAttribArray(4);
   
    // Secondary texture coordinates - lightmaps (location 5)
    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(14 * sizeof(float)));
    glEnableVertexAttribArray(5);
    
    elementCount = indexes.size();
}

// Updated setVertexArray function to handle all attributes
void Mesh::setVertexArray(const std::vector<vec3>& verts,
                          const std::vector<vec3>& normals,
                          const std::vector<vec3>& tangents,
                          const std::vector<vec3>& bitangents,
                          const std::vector<vec2>& texCoords,
                          const std::vector<vec2>& texCoords2) {
    const size_t vertexCount = verts.size();
    const size_t floatsPerVertex = 16;
    const size_t totalFloats = vertexCount * floatsPerVertex;
   
    std::vector<float> rawData;
    rawData.reserve(totalFloats);
   
    auto normalIt = normals.cbegin();
    auto tangentIt = tangents.cbegin();
    auto bitangentIt = bitangents.cbegin();
    auto texCoordIt = texCoords.cbegin();
    auto texCoord2It = texCoords2.cbegin();
   
    for (auto vertIt = verts.cbegin(); vertIt != verts.cend(); ++vertIt) {
        // Position (3 floats)
        rawData.insert(rawData.end(), {vertIt->x, vertIt->y, vertIt->z});
       
        // Normal (3 floats)  
        rawData.insert(rawData.end(), {normalIt->x, normalIt->y, normalIt->z});
       
        // Tangent (3 floats)
        rawData.insert(rawData.end(), {tangentIt->x, tangentIt->y, tangentIt->z});
       
        // Bitangent (3 floats)
        rawData.insert(rawData.end(), {bitangentIt->x, bitangentIt->y, bitangentIt->z});
       
        // Primary texture coordinates (2 floats)
        rawData.insert(rawData.end(), {texCoordIt->s, texCoordIt->t});
       
        // Secondary texture coordinates (2 floats)
        rawData.insert(rawData.end(), {texCoord2It->s, texCoord2It->t});
       
        ++normalIt;
        ++tangentIt;
        ++bitangentIt;
        ++texCoordIt;
        ++texCoord2It;
    }
   
    glBindBuffer(GL_ARRAY_BUFFER, this->gl_VBO);
    glBufferData(GL_ARRAY_BUFFER, totalFloats * sizeof(float), rawData.data(), GL_STATIC_DRAW);
}


unsigned int Mesh::getMaterialIndex(){
    return this->gl_MaterialIndex;
}

void Mesh::setIndexArray(std::vector<unsigned int> idx) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->gl_EBO);
    unsigned int* rawIdx = idx.data();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), rawIdx, GL_STATIC_DRAW);
}

void Mesh::draw(Shader* shader,Material* mat) {
    
    //how will we handle tetures here?
    shader->bindShader();

    glBindVertexArray(this->gl_VAO);
    shader->setUniform(std::string("material" + std::to_string(0)) + ".diffuseColor",mat->diffuseColor); // Texture unit 1


    int unit = 1;
    mat->diffuseMap->bindTexture(unit);
    shader->setUniform(std::string("material" + std::to_string(0)) + ".diffuseMap", unit); // Texture unit 1
    unit++;

    mat->specularMap->bindTexture(unit);
    shader->setUniform(std::string("material" + std::to_string(0)) + ".specularMap", unit); // Texture unit 2
    unit++;

    mat->reflectionMap->bindTexture(unit);
    shader->setUniform(std::string("material" + std::to_string(0)) + ".reflectionMap", unit); // Texture unit 3
    unit++;

    mat->normalMap->bindTexture(unit);
    shader->setUniform(std::string("material" + std::to_string(0)) + ".normalMap", unit); // Texture unit 4
    unit++;

    shader->setUniform(std::string("material" + std::to_string(0)) + ".shininess", mat->shininess);


    



    glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0);
}


