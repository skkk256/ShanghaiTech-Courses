// === ------------------------------------------------------------------------
//
// This file is part of ShanghaiTech CS171.01 Homework. It is \em NOT an open
// source project so students are strongly prohibited from redistributing it,
// i.e. posting it on any public platform, share code with other students.
// Redistributions to the project are considered a severe violation of
// plagiarism regulations.
//
// === -----------------------------------------------------------------------

#ifndef INCLUDE_MESH_H_
#define INCLUDE_MESH_H_

#include <vector>

#include "defines.h"

struct Vertex {
  vec3 position;
  vec3 normal;
};

///1
/// \brief The abstraction of a actual mesh object and its functionalities
/// communicating with OpenGL. You might modify the mesh definition to support
/// persistent data like buffers across frames.
///
/// \note To implement this class, you should understand the concept of
/// - Vertex Array Object (VAO)
/// - Vertex Buffer Object (VBO)
/// - Element Buffer Object (EBO)
/// - And the way to manipulate the OpenGL's large state machine with \em
/// binding and \em unbinding, just like changing the direction of a railway
/// gate. Once changed, the following operations(program flow) will be goes upon
/// the new direction.
///
/// \see https://learnopengl.com/Getting-started/Hello-Triangle
///
/// \todo You might add extra members and functions to this class.
///
class Mesh {
public:
    /// Construct a mesh from an .obj file
    explicit Mesh(const std::string &path);
    Mesh (glm::vec3 center, float radius);
    /// Draw mesh objects
    void draw() const;
    glm::vec3 getCenter() { return center; }
    float getRadius(){ return radius; }

private:
    /// The actual mesh data loaded from the .obj file
    std::vector<Vertex> vertices_;
    std::vector<GLuint> indices_;
    glm::vec3 center;
    float radius;

    unsigned int VAO, VBO, EBO;
    void setupMesh();

    void loadDataFromFile(const std::string &path);
};

#endif  // INCLUDE_MESH_H_
