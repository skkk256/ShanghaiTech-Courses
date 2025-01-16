#ifndef _object_H_
#define _object_H_
#include "defines.h"
#include <shader.h>
#include <vector>

struct Vertex {
  vec3 position;
  vec3 normal;
};

enum FunctionType {
    ARRAY,
    ELEMENTS
};

struct DrawMode {
  GLenum primitive_mode;
};

class Object {
 private:
  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;

 public:
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;

  FunctionType function_type;
  GLenum draw_mode = GL_POINTS;

  Object();
//  explicit Object(std::vector<Vertex> &vertices_);
//  Object(std::vector<Vertex> &vertices_, std::vector<GLuint> &indices_);
  ~Object();

  void init();

  void setDrawMode(GLenum mode);
  void setData(std::vector<Vertex> &vertices_, std::vector<GLuint> &indices_);
  void setData(std::vector<Vertex> &vertices_);
  void setData(std::vector<vec3> &positions_);
  void setData(std::vector<std::vector<vec3>> &positions_);
  void draw() const;
  void draw(const Shader& shader) const;
  void drawArrays() const;
  void drawArrays(const Shader& shader) const;
  void drawElements() const;
  void drawElements(const Shader& shader) const;
};
#endif