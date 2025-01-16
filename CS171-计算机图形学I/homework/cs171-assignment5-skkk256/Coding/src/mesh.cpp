#include <mesh.h>
#include <utils.h>

#include <fstream>
#include <sstream>
#include <vector>

#include "defines.h"

Mesh::Mesh(const std::string &path) { loadDataFromFile(getPath(path)); }

Mesh::Mesh(glm::vec3 center, float radius):center(center), radius(radius) {
    unsigned int longitude = 30;
    unsigned int latitude = 30;
    for (unsigned int y = 0; y <= latitude; ++y) {
        for (unsigned int x = 0; x <= longitude; ++x) {
            float xSegment = (float)x / (float)longitude;
            float ySegment = (float)y / (float)latitude;
            float xPos = radius * std::cos(xSegment * 2.0f * M_PI) * std::sin(ySegment * M_PI);
            float yPos = radius * std::cos(ySegment * M_PI);
            float zPos = radius * std::sin(xSegment * 2.0f * M_PI) * std::sin(ySegment * M_PI);

            Vertex v{};
            v.position = glm::vec3(xPos+center.x, yPos+center.y, zPos+center.z);
            v.normal = glm::vec3(xPos, yPos, zPos);
            vertices_.push_back(v);
//            vertices.push_back(xPos);
//            vertices.push_back(yPos);
//            vertices.push_back(zPos);
        }
    }

    for (unsigned int y = 0; y < latitude; ++y) {
        for (unsigned int x = 0; x < longitude; ++x) {
            indices_.push_back(y * (longitude + 1) + x);
            indices_.push_back((y + 1) * (longitude + 1) + x);
            indices_.push_back((y + 1) * (longitude + 1) + x + 1);

            indices_.push_back(y * (longitude + 1) + x);
            indices_.push_back((y + 1) * (longitude + 1) + x + 1);
            indices_.push_back(y * (longitude + 1) + x + 1);
        }
    }
    setupMesh();

}

void Mesh::setupMesh() {
  // 顶点数据：

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices_.size()*sizeof(Vertex), vertices_.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size()*sizeof(GLuint), indices_.data(), GL_STATIC_DRAW);

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
///
/// \brief Open file with [path]
///
/// \section File format
///
/// For each line starting with 'v' contains 3 floats, which
/// represents the position of a vertex
///
/// For each line starting with 'n' contains 3 floats, which
/// represents the normal of a vertex
///
/// For each line starting with 'f' contains 6 integers
/// [v0, n0, v1, n1, v2, n2], which represents the triangle face
/// v0, n0 means the vertex index and normal index of the first vertex
/// v1, n1 means the vertex index and normal index of the second vertex
/// v2, n2 means the vertex index and normal index of the third vertex
///
/// \param path the path of the obj file
///
/// \section You Task
///
/// \todo Implement mesh data loading from file in this function.
///
/// \param path
void Mesh::loadDataFromFile(const std::string &path) {
    std::cout << "load mesh" << "\n";
    std::ifstream infile(path);
    std::string line;
    std::getline(infile, line);
    std::stringstream ss(line);
    int vertexCount, normalCount, faceCount;
    ss >> vertexCount >> normalCount >> faceCount;
    std::getline(infile, line);

    std::vector<vec3> position_;
    std::vector<vec3> normal_;

    for (int i = 0; i < vertexCount; i++) {
        std::getline(infile, line);
        std::stringstream stringstream(line);
        float x, y, z;
        stringstream >> x >> y >> z;
        Vertex v{};
        v.position = vec3(x/4.0, y/4.0 - 2.0, z/4.0);
        vertices_.push_back(v);
    }
    std::getline(infile, line);
    for (int i = 0; i < normalCount; i++) {
        std::getline(infile, line);
        std::stringstream stringstream(line);
        float x, y, z;
        stringstream >> x >> y >> z;
        normal_.push_back(vec3(x, y, z));
    }
    std::getline(infile, line);
    for (int i = 0; i < faceCount; i++) {
        std::getline(infile, line);
        std::stringstream stringstream(line);
        int p0, n0, p1, n1, p2, n2;
        stringstream >> p0 >> n0 >> p1 >> n1 >> p2 >> n2;
        indices_.push_back(p0);
        vertices_[p0].normal = normal_[n0];
        //      indices_.push_back(n0);
        indices_.push_back(p1);
        vertices_[p1].normal = normal_[n1];

        //      indices_.push_back(n1);
        indices_.push_back(p2);
        vertices_[p2].normal = normal_[n2];
    //      indices_.push_back(n2);
    }
    setupMesh();
}

///
/// \todo Implement draw code here, which is supposed to be invoked in the main
/// loop.
///
void Mesh::draw() const {
  // 绘制网格
  // 1st attribute buffer : vertices
  glBindVertexArray(VAO);
  //glDrawArrays(GL_TRIANGLES, 0, 6);
//  glDrawArrays(GL_TRIANGLES, 0, vertices_.size());
  glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}