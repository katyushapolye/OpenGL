#include "../headers/Shader.h"

Shader::Shader() {
    this->gl_VertexShaderID   = glCreateShader(GL_VERTEX_SHADER);
    this->gl_GeometryShaderID = 0; // 0 = not used
    this->gl_FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    this->gl_ShaderProgramID  = 0;
    this->gl_GeometryShaderID = -1;
}

bool Shader::compileShader() {
    int  success;
    char infoLog[512];

    glCompileShader(this->gl_VertexShaderID);
    glGetShaderiv(this->gl_VertexShaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(this->gl_VertexShaderID, 512, NULL, infoLog);
        Log::write("[Shader::compileShader] VERTEX ERROR: " + std::string(infoLog));
        return false;
    }

    // Only compile geometry shader if one was actually loaded
    if (this->gl_GeometryShaderID != -1) {
        glCompileShader(this->gl_GeometryShaderID);
        glGetShaderiv(this->gl_GeometryShaderID, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(this->gl_GeometryShaderID, 512, NULL, infoLog);
            Log::write("[Shader::compileShader] GEOMETRY ERROR: " + std::string(infoLog));
            return false;
        }
    }

    glCompileShader(this->gl_FragmentShaderID);
    glGetShaderiv(this->gl_FragmentShaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(this->gl_FragmentShaderID, 512, NULL, infoLog);
        Log::write("[Shader::compileShader] FRAGMENT ERROR: " + std::string(infoLog));
        return false;
    }

    return true;
}

bool Shader::loadFromString(std::string vertexCode, std::string fragmentCode) {
    // No geometry shader for this overload
    this->gl_GeometryShaderID = -1;


    const char* rawCode = vertexCode.c_str();
    glShaderSource(this->gl_VertexShaderID, 1, &rawCode, NULL);

    rawCode = fragmentCode.c_str();
    glShaderSource(this->gl_FragmentShaderID, 1, &rawCode, NULL);

    if (!compileShader()) return false;

    this->gl_ShaderProgramID = glCreateProgram();
    glAttachShader(this->gl_ShaderProgramID, this->gl_VertexShaderID);
    glAttachShader(this->gl_ShaderProgramID, this->gl_FragmentShaderID);
    glLinkProgram(this->gl_ShaderProgramID);

    int  success;
    char infoLog[512];
    glGetProgramiv(this->gl_ShaderProgramID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(this->gl_ShaderProgramID, 512, NULL, infoLog);
        Log::write("[Shader::loadFromString] LINK ERROR: " + std::string(infoLog));
        return false;
    }

    glDeleteShader(this->gl_VertexShaderID);
    glDeleteShader(this->gl_FragmentShaderID);
    return true;
}

bool Shader::loadFromString(std::string vertexCode, std::string geometryCode, std::string fragmentCode) {
    

    this->gl_GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);

    const char* rawCode = vertexCode.c_str();
    glShaderSource(this->gl_VertexShaderID, 1, &rawCode, NULL);

    rawCode = geometryCode.c_str();
    glShaderSource(this->gl_GeometryShaderID, 1, &rawCode, NULL);

    rawCode = fragmentCode.c_str();
    glShaderSource(this->gl_FragmentShaderID, 1, &rawCode, NULL);

    if (!compileShader()) return false;

    this->gl_ShaderProgramID = glCreateProgram();
    glAttachShader(this->gl_ShaderProgramID, this->gl_VertexShaderID);
    glAttachShader(this->gl_ShaderProgramID, this->gl_GeometryShaderID);
    glAttachShader(this->gl_ShaderProgramID, this->gl_FragmentShaderID);
    glLinkProgram(this->gl_ShaderProgramID);

    int  success;
    char infoLog[512];
    glGetProgramiv(this->gl_ShaderProgramID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(this->gl_ShaderProgramID, 512, NULL, infoLog);
        Log::write("[Shader::loadFromString] LINK ERROR: " + std::string(infoLog));
        return false;
    }

    glDeleteShader(this->gl_VertexShaderID);
    glDeleteShader(this->gl_GeometryShaderID);
    glDeleteShader(this->gl_FragmentShaderID);
    return true;
}

bool Shader::loadFromFile(std::string vertexPath, std::string fragmentPath) {
    Log::write("[Shader] Loading: " + vertexPath + " | " + fragmentPath);

    std::string vertexCode, fragmentCode;
    std::ifstream vShaderFile, fShaderFile;
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vss, fss;
        vss << vShaderFile.rdbuf();
        fss << fShaderFile.rdbuf();
        vertexCode   = vss.str();
        fragmentCode = fss.str();
    }
    catch (const std::ifstream::failure& e) {
        Log::write("[Shader] File read failed: " + std::string(e.what()));
        return false;
    }

    return loadFromString(vertexCode, fragmentCode);
}

bool Shader::loadFromFile(std::string vertexPath, std::string geometryPath, std::string fragmentPath) {
    Log::write("[Shader] Loading (geo): " + vertexPath + " | " + geometryPath + " | " + fragmentPath);

    std::string vertexCode, geometryCode, fragmentCode;
    std::ifstream vShaderFile, gShaderFile, fShaderFile;
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vShaderFile.open(vertexPath);
        gShaderFile.open(geometryPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vss, gss, fss;
        vss << vShaderFile.rdbuf();
        gss << gShaderFile.rdbuf();
        fss << fShaderFile.rdbuf();
        vertexCode   = vss.str();
        geometryCode = gss.str();
        fragmentCode = fss.str();
    }
    catch (const std::ifstream::failure& e) {
        Log::write("[Shader] File read failed: " + std::string(e.what()));
        return false;
    }

    return loadFromString(vertexCode, geometryCode, fragmentCode);
}

void Shader::bindShader() {
    glUseProgram(this->gl_ShaderProgramID);
}

void Shader::setUniform(std::string name, vec2 value) {
    glUniform2f(glGetUniformLocation(this->gl_ShaderProgramID, name.c_str()), value.x, value.y);
}
void Shader::setUniform(std::string name, vec3 value) {
    glUniform3f(glGetUniformLocation(this->gl_ShaderProgramID, name.c_str()), value.x, value.y, value.z);
}
void Shader::setUniform(std::string name, vec4 value) {
    glUniform4f(glGetUniformLocation(this->gl_ShaderProgramID, name.c_str()), value.x, value.y, value.z, value.w);
}
void Shader::setUniform(std::string name, float value) {
    glUniform1f(glGetUniformLocation(this->gl_ShaderProgramID, name.c_str()), value);
}
void Shader::setUniform(std::string name, int value) {
    glUniform1i(glGetUniformLocation(this->gl_ShaderProgramID, name.c_str()), value);
}
void Shader::setUniform(std::string name, bool value) {
    glUniform1i(glGetUniformLocation(this->gl_ShaderProgramID, name.c_str()), value);
}
void Shader::setUniform(std::string name, mat4 value) {
    glUniformMatrix4fv(glGetUniformLocation(this->gl_ShaderProgramID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setUniform(std::string name, mat3 value) {
    glUniformMatrix3fv(glGetUniformLocation(this->gl_ShaderProgramID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}