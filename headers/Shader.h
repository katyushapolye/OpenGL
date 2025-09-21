#ifndef SHADER_H
#define SHADER_H

#include "Definitions.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {


private:
	int gl_VertexShaderID;
	int gl_FragmentShaderID;
	int gl_ShaderProgramID;

	bool compileShader();
public:
	Shader();

	void loadFromString(std::string vertexCode,std::string fragmentCode);
	void loadFromFile(std::string vertexPath, std::string fragmentPath);

	void bindShader();



	void setUniform(std::string name,vec2 value);
	void setUniform(std::string name,vec3 value);
	void setUniform(std::string name,vec4 value);
	void setUniform(std::string name,int value);
	void setUniform(std::string name,float value);
	void setUniform(std::string name,mat4 value);
	void setUniform(std::string name,mat3 value);
};

#endif
