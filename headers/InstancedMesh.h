//encapsulates a VAO wth a VBO and EBO. Only that and nothing more
#ifndef INSTANCEDMESH_H

#define INSTANCEDMESH_H



#include <iostream>
#include <vector>
#include "Definitions.h"
#include "Shader.h"
#include "Texture.h"
#include "Material.h"



class InstancedMesh {
private:
    unsigned int gl_VAO,gl_EBO,gl_VBO;
    unsigned int gl_Instance_VBO;
    unsigned int gl_MaterialIndex;

    int elementCount;
    std::vector<Vertex> gl_Vertices;
    std::vector<unsigned int> gl_Indices; //ebos indexes



    // Private helper method to initialize the mesh
    void initializeInstancedMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indexes);

public:
    //holds the standard mesh VAO that is used to render all meshes



    unsigned int getMaterialIndex();

    InstancedMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indexes,unsigned int materialIndex);
    
    void setInstanceMatrixArray(std::vector<mat4> instanceMatrices);


    void InstancedDraw(Shader* shader,Material* mat,std::vector<mat4>& instanceMatrices);


};




#endif