#include "../headers/Mesh.h"


//we need to update intilize
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indexes,unsigned int materialIndex) {
    std::vector<vec3> verts;
    std::vector<vec3> normals;
    std::vector<vec2> texCoords;
    this->gl_MaterialIndex = materialIndex;
    

    
    
    for (const auto& vertex : vertices) {
        verts.push_back(vertex.position);
        normals.push_back(vertex.normal);
        texCoords.push_back(vertex.texCoords);
    }
    
    initializeMesh(verts, normals, texCoords, indexes);
    this->gl_Vertices = vertices;
    this->gl_Indices = indexes;

}



void Mesh::initializeMesh(std::vector<vec3>& verts, std::vector<vec3>& normals, std::vector<vec2>& texCoords, std::vector<unsigned int>& indexes) {
    this->gl_VBO = 0;
    this->gl_VAO = 0;
    this->gl_EBO = 0;
    
    glGenVertexArrays(1, &this->gl_VAO);  
    glGenBuffers(1, &this->gl_VBO);      
    glGenBuffers(1, &this->gl_EBO);
    glBindVertexArray(this->gl_VAO); // initial binding
    
    setVertexArray(verts, normals, texCoords);
    setIndexArray(indexes);

    //our attr for now
    /*vec3 pos
      vec3 normal
      vec3 tangent
      vec3 bitangent
      vec2 texCoords;
      vec texCoords2; //lightmaps

    */
    
    // Position attribute (location 0)
                //location, sizeof, type, normalize, stride, alwayszero
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


void Mesh::setVertexArray(const std::vector<vec3>& verts, 
                          const std::vector<vec3>& normals, 
                          const std::vector<vec2>& texCoords) {
    const size_t vertexCount = verts.size();
    const size_t floatsPerVertex = 16;
    const size_t totalFloats = vertexCount * floatsPerVertex;
    

    std::vector<float> rawData;
    rawData.reserve(totalFloats);
    

    auto normalIt = normals.cbegin();
    auto texCoordIt = texCoords.cbegin();
    
    for (auto vertIt = verts.cbegin(); vertIt != verts.cend(); ++vertIt) {
        // Position (3 floats)
        rawData.insert(rawData.end(), {vertIt->x, vertIt->y, vertIt->z});
        
        // Normal (3 floats)  
        rawData.insert(rawData.end(), {normalIt->x, normalIt->y, normalIt->z});
        
        // Tangent (3 floats) - zeros for now
        rawData.insert(rawData.end(), {0.0f, 0.0f, 0.0f});
        
        // Bitangent (3 floats) - zeros for now
        rawData.insert(rawData.end(), {0.0f, 0.0f, 0.0f});
        
        // Primary texture coordinates (2 floats)
        rawData.insert(rawData.end(), {texCoordIt->s, texCoordIt->t});
        
        // Secondary texture coordinates (2 floats) - zeros for now
        rawData.insert(rawData.end(), {0.0f, 0.0f});
        
        ++normalIt;
        ++texCoordIt;
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
    //shader.bindShader();
    int unit = 0;


            mat->diffuse->bindTexture(unit);
            shader->setUniform(std::string("material" + std::to_string(0)) + ".diffuse", unit); // Texture unit 0
            unit++;

            mat->specular->bindTexture(unit);
            shader->setUniform(std::string("material" + std::to_string(0)) + ".specular", unit); // Texture unit 0  
            unit++;

            mat->normal->bindTexture(unit);
            shader->setUniform(std::string("material" + std::to_string(0)) + ".normal", unit); // Texture unit 0  
            unit++;

            shader->setUniform(std::string("material" + std::to_string(0)) + ".shininess", mat->shininess);
        

    



    glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0);
}


