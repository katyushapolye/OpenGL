#include "../headers/Shader.h"

//change it so we handle all shaders in one object 
Shader::Shader() {


		this->gl_VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
		this->gl_FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	

}

bool Shader::compileShader(){
	glCompileShader(this->gl_VertexShaderID);
	int success;
    char infoLog[512];
    glGetShaderiv(this->gl_VertexShaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(this->gl_VertexShaderID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		return false;
    }

	glCompileShader(this->gl_FragmentShaderID);
    glGetShaderiv(this->gl_FragmentShaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(this->gl_FragmentShaderID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		return false;
    }
	return true;
}

bool Shader::loadFromString(std::string vertexCode,std::string fragmentCode) {
	const char* rawCode = vertexCode.c_str();
	glShaderSource(this->gl_VertexShaderID,1,&rawCode,NULL);
	if(compileShader() == false){
		return false;
	};

	rawCode = fragmentCode.c_str();
	glShaderSource(this->gl_FragmentShaderID,1,&rawCode,NULL);
	if(compileShader() == false){
		return false;
	};

	//we compiled both sucefully so we can make a program now

	this->gl_ShaderProgramID = glCreateProgram();
	glAttachShader(this->gl_ShaderProgramID,this->gl_VertexShaderID);
	glAttachShader(this->gl_ShaderProgramID,this->gl_FragmentShaderID);
	glLinkProgram(this->gl_ShaderProgramID);

	//check for linkage errors
	int success;
	 char infoLog[512];
	glGetProgramiv(this->gl_ShaderProgramID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(this->gl_ShaderProgramID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return false;
    }
    glDeleteShader(this->gl_VertexShaderID);
    glDeleteShader(this->gl_FragmentShaderID);

    return true;

}


bool Shader::loadFromFile(std::string vertexPath, std::string fragmentPath){
	std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try 
    {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();		
        vShaderFile.close();
        fShaderFile.close();
        vertexCode   = vShaderStream.str();
        fragmentCode = fShaderStream.str();		
    }
    catch(std::ifstream::failure* e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

	return loadFromString(vertexCode,fragmentCode);
}



void Shader::bindShader(){

	glUseProgram(this->gl_ShaderProgramID);
	
}

void Shader::setUniform(std::string name,vec2 value){
	glUniform2f(glGetUniformLocation(this->gl_ShaderProgramID,name.c_str()),value.x,value.y);
}

void Shader::setUniform(std::string name,vec3 value){
	glUniform3f(glGetUniformLocation(this->gl_ShaderProgramID,name.c_str()),value.x,value.y,value.z);
}

void Shader::setUniform(std::string name,vec4 value){
	glUniform4f(glGetUniformLocation(this->gl_ShaderProgramID,name.c_str()),value.x,value.y,value.z,value.w);
}

void Shader::setUniform(std::string name,float value){
	glUniform1f(glGetUniformLocation(this->gl_ShaderProgramID,name.c_str()),value);
}

void Shader::setUniform(std::string name,int value){
	glUniform1i(glGetUniformLocation(this->gl_ShaderProgramID,name.c_str()),value);
}

void Shader::setUniform(std::string name,bool value){
	glUniform1i(glGetUniformLocation(this->gl_ShaderProgramID,name.c_str()),value);
}

void Shader::setUniform(std::string name,mat4 value){
    
    glUniformMatrix4fv(glGetUniformLocation(this->gl_ShaderProgramID,name.c_str()),1,GL_FALSE,glm::value_ptr(value));
}
void Shader::setUniform(std::string name,mat3 value){
    glUniformMatrix3fv(glGetUniformLocation(this->gl_ShaderProgramID,name.c_str()),1,GL_FALSE,glm::value_ptr(value));
}


