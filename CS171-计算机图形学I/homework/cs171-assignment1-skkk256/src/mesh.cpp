#include <mesh.h>
#include <utils.h>

#include <fstream>
#include <sstream>
#include <vector>

Mesh::Mesh(const std::string &path) { loadDataFromFile(getPath(path)); }

void Mesh::setupMesh() {
  // 顶点数据：
  GLfloat vertices[] = {
      0.5f,  0.5f, 0.0f,  // Top Right
      0.5f, -0.5f, 0.0f,  // Bottom Right
      -0.5f, -0.5f, 0.0f,  // Bottom Left
      -0.5f,  0.5f, 0.0f   // Top Left
  };
  GLuint indices[] = {  // Note that we start from 0!
      0, 1, 3,  // First Triangle
      1, 2, 3   // Second Triangle
  };

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


  glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
  glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
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
    v.position = vec3(x, y, z);
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
//      Vertex v0{position_[p0], normal_[n0]};
//      vertices_.push_back(v0);
//      Vertex v1{position_[p1], normal_[n1]};
//      vertices_.push_back(v1);
//      Vertex v2{position_[p2], normal_[n2]};
//      vertices_.push_back(v2);
//      Vertex vec{position_[p0], normal_[n0]};
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
//std::cout << vertices_.size() << std::endl;
//print vertices
//  for (int i = 0; i < vertices_.size(); i++) {
//      std::cout << vertices_[i].position.x << "f, " << vertices_[i].position.y
//                << "f, " << vertices_[i].position.z << "f,  ";
//      std::cout << vertices_[i].normal.x << "f, " << vertices_[i].normal.y << "f, "
//                << vertices_[i].normal.z << "f, " << std::endl;
//  }
//  //print index
//  for (int i = 0; i < indices_.size(); i++) {
//      std::cout << indices_[i] << " ";
//  }
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