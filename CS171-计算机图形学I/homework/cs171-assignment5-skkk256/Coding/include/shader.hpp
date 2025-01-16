#pragma once

#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader {
public:
    unsigned int id;

    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);

    // activate the shader
    void use() const { glUseProgram(id);}
    void setBool(const std::string &name, bool value) const {
        glUniform1i(getUniformLocation(name), (int)value);
    }
    void setInt(const std::string &name, int value) const {
        glUniform1i(getUniformLocation(name), value);
    }
    void setFloat(const std::string &name, float value) const {
        glUniform1f(getUniformLocation(name), value);
    }
    void setVec2(const std::string &name, const glm::vec2 &value) const {
        glUniform2fv(getUniformLocation(name), 1, &value[0]);
    }
    void setVec2(const std::string &name, float x, float y) const {
        glUniform2f(getUniformLocation(name), x, y);
    }
    void setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(getUniformLocation(name), 1, &value[0]);
    }
    void setVec3(const std::string &name, float x, float y, float z) const {
        glUniform3f(getUniformLocation(name), x, y, z);
    }
    void setVec4(const std::string &name, const glm::vec4 &value) const {
        glUniform4fv(getUniformLocation(name), 1, &value[0]);
    }
    void setVec4(const std::string &name, float x, float y, float z, float w) const {
        glUniform4f(getUniformLocation(name), x, y, z, w);
    }
    void setMat2(const std::string &name, const glm::mat2 &mat) const {
        glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
    void setMat3(const std::string &name, const glm::mat3 &mat) const {
        glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
private:
    GLint getUniformLocation(const std::string &name) const {
        return glGetUniformLocation(this->id, name.c_str());
    }
    // utility function for checking shader compilation/linking errors.
    void checkCompileErrors(GLuint shader, std::string type);
};