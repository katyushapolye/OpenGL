#include "../headers/Mesh.h"



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


void Mesh::setVertexArray(std::vector<vec3> verts, std::vector<vec3> normals, std::vector<vec2> texCoords) {
    glBindBuffer(GL_ARRAY_BUFFER, this->gl_VBO);
    float* rawData = new float[verts.size() * 16];  // 16 floats per vertex now
    int c = 0;
   
    auto gl_Normal = normals.begin();
    auto gl_TexCoord = texCoords.begin();
   
    for (auto gl_Pos = verts.begin(); gl_Pos != verts.end(); gl_Pos++) {
        // Position (3 floats)
        rawData[c] = gl_Pos->x;
        rawData[c + 1] = gl_Pos->y;
        rawData[c + 2] = gl_Pos->z;
        
        // Normal (3 floats)
        rawData[c + 3] = gl_Normal->x;
        rawData[c + 4] = gl_Normal->y;
        rawData[c + 5] = gl_Normal->z;
        
        // Tangent (3 floats) - zeros for now
        rawData[c + 6] = 0.0f;
        rawData[c + 7] = 0.0f;
        rawData[c + 8] = 0.0f;
        
        // Bitangent (3 floats) - zeros for now
        rawData[c + 9] = 0.0f;
        rawData[c + 10] = 0.0f;
        rawData[c + 11] = 0.0f;
        
        // Primary texture coordinates (2 floats)
        rawData[c + 12] = gl_TexCoord->s;
        rawData[c + 13] = gl_TexCoord->t;
        
        // Secondary texture coordinates (2 floats) - zeros for now
        rawData[c + 14] = 0.0f;
        rawData[c + 15] = 0.0f;
       
        gl_Normal++;
        gl_TexCoord++;
        c += 16;  // Move to next vertex (16 floats ahead)
    }
   
    glBufferData(GL_ARRAY_BUFFER, verts.size() * 16 * sizeof(float), rawData, GL_STATIC_DRAW);
    delete[] rawData;
}


unsigned int Mesh::getMaterialIndex(){
    return this->gl_MaterialIndex;
}

void Mesh::setIndexArray(std::vector<unsigned int> idx) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->gl_EBO);
    unsigned int* rawIdx = idx.data();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), rawIdx, GL_STATIC_DRAW);
}

void Mesh::draw(const std::unique_ptr<Shader>& shader,std::unique_ptr<Material>& mat) {
    
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


