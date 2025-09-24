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


    void setVertexArray(const std::vector<vec3>& verts,
                   const std::vector<vec3>& normals,
                   const std::vector<vec3>& tangents,
                   const std::vector<vec3>& bitangents,
                   const std::vector<vec2>& texCoords,
                   const std::vector<vec2>& texCoords2);
    void setIndexArray(std::vector<unsigned int> idx);
    
    // Private helper method to initialize the mesh
    void initializeMesh(std::vector<vec3>& verts, 
                   std::vector<vec3>& normals, 
                   std::vector<vec3>& tangents,
                   std::vector<vec3>& bitangents,
                   std::vector<vec2>& texCoords,
                   std::vector<vec2>& texCoords2,
                   std::vector<unsigned int>& indexes);

public:
    //holds the standard mesh VAO that is used to render all meshes



    unsigned int getMaterialIndex();

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indexes,unsigned int materialIndex);



    void draw(Shader* shader,Material* mat);


};




#endif