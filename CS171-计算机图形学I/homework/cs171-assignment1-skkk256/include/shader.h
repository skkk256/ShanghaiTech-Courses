// === ------------------------------------------------------------------------
//
// This file is part of ShanghaiTech CS171.01 Homework. It is \em NOT an open
// source project so students are strongly prohibited from redistributing it,
// i.e. posting it on any public platform, share code with other students.
// Redistributions to the project are considered a severe violation of
// plagiarism regulations.
//
// === -----------------------------------------------------------------------

#ifndef INCLUDE_SHADER_H_
#define INCLUDE_SHADER_H_

#include <string>

#include "defines.h"

///
/// \brief The abstraction of shader programs (including vertex shader and the
/// fragment shader).
///
/// \note A shader program is a piece of code that runs on GPU. Note the
/// significant difference between a shader and a C++ program. Shader is
/// <em>loaded and compiled at runtime</em> and thus can be modified on-the-fly.
///
/// \see A list of readings on shader
/// - https://en.wikipedia.org/wiki/Shader
/// - https://learnopengl.com/Getting-started/Shaders
///
class Shader {
 public:
  Shader() = default;

  /// Construct the shaders with shader files
  Shader(const std::string &vsPath, const std::string &fsPath, const std::string &gsPath=nullptr);

  ///
  /// \brief Load and use the shader program.
  ///
  /// \note "OpenGL is by itself a large state machine: a collection of
  /// variables that define how OpenGL should currently operate.". One great
  /// view of OpenGL is a train running on a track, where you operations like
  /// invoking the glUseProgram changes the railway gates, leading the train to
  /// a different track (i.e., rendering by a different shader program, or
  /// waiting for you to specify some more transitions).
  ///
  /// \see glUseProgram
  /// \see https://learnopengl.com/Getting-started/OpenGL
  ///
  void use() const;

  /// Get a uniform variable's location according to its name
  [[nodiscard]] GLint getUniformLocation(const std::string &name) const;

  ///
  /// \brief Set a uniform variable's value
  ///
  /// \note Shaders come with a set of input variables to communicate.
  ///
  GLuint getID() const;
  void setInt(const std::string &name, GLint value) const;
  void setFloat(const std::string &name, GLfloat value) const;
  void setMat3(const std::string &name, const mat3 &value) const;
  void setMat4(const std::string &name, const mat4 &value) const;
  void setVec3(const std::string &name, const vec3 &value) const;
  void setVec4(const std::string &name, const vec4 &value) const;

 private:
  GLuint id_ = 0;

  void init(const std::string &vsPath, const std::string &fsPath, const std::string &gsPath);

  /// Load the shader code from a file
  static std::string getCodeFromFile(const std::string &path);

  void checkCompileErrors(GLuint shader, std::string type);
  ///
  /// \brief Init shader with shader codes
  ///
  /// \param vs The vertex shader code content
  /// \param fs The fragment shader code content
  ///
  void initWithCode(const std::string &vs, const std::string &fs, const std::string &gs);
};

#endif  // INCLUDE_SHADER_H_
