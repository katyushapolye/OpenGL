//encapsulates a VAO wth a VBO and EBO. Only that and nothing more
#ifndef MESH_H

#define MESH_H



#include <iostream>
#include <vector>
#include "Definitions.h"
#include "Shader.h"
#include "Texture.h"
#include "Material.h"



class Mesh {
private:
    unsigned int gl_VBO, gl_VAO, gl_EBO;

    unsigned int gl_MaterialIndex;

    int elementCount;
    std::vector<Vertex> gl_Vertices;
    std::vector<unsigned int> gl_Indices; //ebos indexes


    void setVertexArray(std::vector<vec3> verts, std::vector<vec3> normals, std::vector<vec2> texCoords);
    void setIndexArray(std::vector<unsigned int> idx);
    
    // Private helper method to initialize the mesh
    void initializeMesh(std::vector<vec3>& verts, std::vector<vec3>& normals, std::vector<vec2>& texCoords, std::vector<unsigned int>& indexes);

public:
    // Constructor overloads

    unsigned int getMaterialIndex();

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indexes,unsigned int materialIndex);



    void draw(const std::unique_ptr<Shader>& shader,std::unique_ptr<Material>& mat);


};


#endif