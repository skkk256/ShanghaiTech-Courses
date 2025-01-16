#include <shader.h>
#include <utils.h>

#include <fstream>
#include <sstream>

Shader::Shader(const std::string &vsPath, const std::string &fsPath, const std::string &gsPath) {
  init(vsPath, fsPath, gsPath);
}

void Shader::init(const std::string &vsPath, const std::string &fsPath, const std::string &gsPath) {
  if (gsPath.empty()) {
        std::cout << "No geometry shader" << std::endl;
        initWithCode(getCodeFromFile(vsPath), getCodeFromFile(fsPath), "");
        std::cout << "No geometry shader init" << std::endl;
    }
    else {
        std::cout << "Geometry shader" << std::endl;
        initWithCode(getCodeFromFile(vsPath), getCodeFromFile(fsPath), getCodeFromFile(gsPath));
        std::cout << "Geometry shader init" << std::endl;
  }
}
///
/// \brief Init shader with shader codes
///
/// \param vs The vertex shader code content
/// \param fs The fragment shader code content
///
/// \todo Implement this function to set shaders to OpenGL
///
void Shader::initWithCode(const std::string &vs, const std::string &fs, const std::string &gs) {

  char const * vertexShaderSource = vs.c_str();
  char const * fragmentShaderSource = fs.c_str();
  char const  * geometryShaderSource = nullptr;
  GLuint geometryShader = 0;
  if (!gs.empty())
      geometryShaderSource = gs.c_str();

  // Build and compile our shader program
  // Vertex shader
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  // Check for compile time errors
  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  // Fragment shader
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  // Check for compile time errors
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  // Geometry shader
  if (!gs.empty()) {
    geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometryShader, 1, &geometryShaderSource, NULL);
    glCompileShader(geometryShader);
    // Check for compile time errors
    glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                << infoLog << std::endl;
    }
  }

  // Link shaders
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  if (!gs.empty())
    glAttachShader(shaderProgram, geometryShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  // Check for linking errors
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  id_ = shaderProgram;
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

void Shader::use() const { glUseProgram(id_); }

GLuint Shader::getID() const { return id_; }

GLint Shader::getUniformLocation(const std::string &name) const {
  return glGetUniformLocation(id_, name.c_str());
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

void Shader::setVec3(const std::string &name, const vec3 &value) const {
  glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string &name, const vec4 &value) const {
  glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}
