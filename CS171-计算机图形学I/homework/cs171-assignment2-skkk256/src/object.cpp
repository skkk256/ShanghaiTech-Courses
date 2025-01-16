#include <object.h>
#include <shader.h>
#include <utils.h>
#include <fstream>
#include <vector>

Object::Object() {}
Object::~Object() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
}

//Object::Object(std::vector<Vertex> &vertices_) {
//  this->setData(vertices_);
//}
//
//Object::Object(std::vector<Vertex> &vertices_, std::vector<GLuint> &indices_) {
//  this->setData(vertices_, indices_);
//}

/**
 * TODO: initialize VAO, VBO, VEO and set the related varialbes
 */
void Object::setData(std::vector<Vertex> &vertices_, std::vector<GLuint> &indices_) {
  vertices = vertices_;
  indices = indices_;
  function_type = ELEMENTS;
  init();
}

void Object::setData(std::vector<Vertex> &vertices_) {
  vertices = vertices_;
  function_type = ARRAY;
  init();
}

void Object::setData(std::vector<vec3> &positions_) {
  vertices.resize(positions_.size());
  for (int i = 0; i < positions_.size(); ++i) {
    vertices[i].position = positions_[i];
    vertices[i].normal = {0, 0, 0};
  }
  function_type = ARRAY;
  init();
}

void Object::setData(std::vector<std::vector<vec3>> &positions_) {
  vertices.resize(positions_.size() * positions_[0].size());
  for (int i = 0; i < positions_.size(); ++i) {
    for (int j = 0; j < positions_[0].size(); ++j) {
      vertices[i * positions_[0].size() + j].position = positions_[i][j];
      vertices[i * positions_[0].size() + j].normal = {0, 0, 0};
    }
  }
  function_type = ARRAY;
  init();
}

void Object::setDrawMode(GLenum mode) {
  draw_mode = mode;
}

void Object::init() {
//  glGenVertexArrays(1, &VAO);
//  glGenBuffers(1, &VBO);
//
//  glBindVertexArray(VAO);
//
//  glBindBuffer(GL_ARRAY_BUFFER, VBO);
//  glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
//
//  // 设置顶点坐标指针
//  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
//                        (GLvoid*)nullptr);
//  glEnableVertexAttribArray(0);
//
//  // 设置法线指针
//  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
//                        (GLvoid*)offsetof(Vertex, normal));
//  glEnableVertexAttribArray(1);
//
//  glBindBuffer(GL_ARRAY_BUFFER, 0);
//  glBindVertexArray(0);
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

  // 设置顶点坐标指针
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid*)0);
  glEnableVertexAttribArray(0);

  // 设置法线指针
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid*)offsetof(Vertex, normal));
  glEnableVertexAttribArray(1);


  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

/**
 * TODO: draw object with VAO and VBO
 * You can choose to implement either one or both of the following functions.
 */

/* Implement this one if you do not use a shader */
void Object::drawArrays() const {
  glBindVertexArray(VAO);
  glDrawArrays(draw_mode, 0, vertices.size());
  glBindVertexArray(0);
}

/* Implement this one if you do use a shader */
void Object::drawArrays(const Shader& shader) const {
  // !DELETE THIS WHEN YOU FINISH THIS FUNCTION
  UNIMPLEMENTED;
}

/**
 * TODO: draw object with VAO, VBO, and VEO
 * You can choose to implement either one or both of the following functions.
 */

/* Implement this one if you do not use a shader */
void Object::drawElements() const {
  // !DELETE THIS WHEN YOU FINISH THIS FUNCTION
  glBindVertexArray(VAO);
  glDrawElements(draw_mode, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

/* Implement this one if you do use a shader */
void Object::drawElements(const Shader& shader) const {
  // !DELETE THIS WHEN YOU FINISH THIS FUNCTION
  shader.use();
  glBindVertexArray(VAO);
  glDrawElements(draw_mode, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void Object::draw() const {
  if (function_type == ARRAY) {
    drawArrays();
  } else if (function_type == ELEMENTS) {
    drawElements();
  }
}

void Object::draw(const Shader& shader) const {
  if (function_type == ARRAY) {
    drawArrays(shader);
  } else if (function_type == ELEMENTS) {
    drawElements(shader);
  }
}