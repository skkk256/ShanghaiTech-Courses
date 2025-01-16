#include <shader.h>
#include <utils.h>
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const std::string &vsPath, const std::string &fsPath) {
  init(vsPath, fsPath);
}

void Shader::init(const std::string &vsPath, const std::string &fsPath) {
  initWithCode(getCodeFromFile(vsPath), getCodeFromFile(fsPath));
}

void Shader::initWithCode(const std::string &vs, const std::string &fs) {
  GLuint vertexShader, fragmentShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  const GLchar *vsCode = vs.c_str();
  glShaderSource(vertexShader, 1, &vsCode, nullptr);
  glCompileShader(vertexShader);
  int success;
  char infoLog[1024];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  const GLchar *fsCode = fs.c_str();
  glShaderSource(fragmentShader, 1, &fsCode, nullptr);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success)   {
    glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  id = glCreateProgram();
  glAttachShader(id, vertexShader);
  glAttachShader(id, fragmentShader);
  glLinkProgram(id);
  glGetProgramiv(id, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(id, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

std::string Shader::getCodeFromFile(const std::string &path) {
  std::string code;
  std::ifstream file;
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    file.open(getPath(path));
    std::stringstream stream;
    stream << file.rdbuf();
    file.close();
    code = stream.str();
  } catch (std::ifstream::failure &e) {
    LOG_ERR("File Error: " + std::string(e.what()));
  }
  return code;
}

void Shader::use() const { glUseProgram(id); }

GLint Shader::getUniformLocation(const std::string &name) const {
  return glGetUniformLocation(id, name.c_str());
}

void Shader::setInt(const std::string &name, GLint value) const {
  glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const std::string &name, GLfloat value) const {
  glUniform1f(getUniformLocation(name), value);
}

void Shader::setMat3(const std::string &name, const mat3 &value) const {
  glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE,
                     glm::value_ptr(value));
}

void Shader::setMat4(const std::string &name, const mat4 &value) const {
  glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE,
                     glm::value_ptr(value));
}

void Shader::setVec2(const std::string &name, const vec2 &value) const {
  glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string &name, const vec3 &value) const {
  glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string &name, const vec4 &value) const {
  glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}