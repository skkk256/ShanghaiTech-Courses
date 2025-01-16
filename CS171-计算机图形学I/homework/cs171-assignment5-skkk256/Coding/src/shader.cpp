#include "shader.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(
    const char* _vertexPath,
    const char* _fragmentPath,
    const char* _geometryPath
) {
    // retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;

    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try  {
        // open files
        vShaderFile.open(_vertexPath);
        fShaderFile.open(_fragmentPath);

        std::stringstream vShaderStream, fShaderStream, gShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        // close file handlers
        vShaderFile.close();
        fShaderFile.close();

        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        if (_geometryPath) {
            gShaderFile.open(_geometryPath);
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char * fShaderCode = fragmentCode.c_str();
    const char* gShaderCode = geometryCode.c_str();

    // compile shaders
    unsigned int vertex, fragment, geometry;
    int success;
    char infoLog[512];

    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    // geometry shader
    if (_geometryPath != NULL) {
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        checkCompileErrors(geometry, "Geometry");
    }

    // shader Program
    this->id = glCreateProgram();
    glAttachShader(this->id, vertex);
    glAttachShader(this->id, fragment);
    if (_geometryPath != NULL) {
        glAttachShader(this->id, geometry);
    }
    glLinkProgram(this->id);
    checkCompileErrors(this->id, "PROGRAM");

    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (_geometryPath != NULL) {
        glDeleteShader(geometry);
    }
}

void Shader::
checkCompileErrors(GLuint _shader, std::string _type)
{
    GLint success;
    GLchar infoLog[1024];
    if (_type != "PROGRAM") {
        glGetShaderiv(_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(_shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << _type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            // std::cin.get();
            exit(-1);
        }
    }
    else {
        glGetProgramiv(_shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(_shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << _type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            // std::cin.get();
            exit(-1);
        }
    }
}