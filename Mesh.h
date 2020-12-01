#ifndef MESH_H
#define MESH_H

#include "glm/glm.hpp"
#include <vector>
#include <iostream>

// Based on Learn OpenGL's mesh loading
struct Vertex{
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

struct Texture {
  unsigned int id;
  std::string type;
};
class Mesh{
public:
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;
  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
  void Draw(int shader, unsigned int drawMode);
protected:
  unsigned int VBO, VAO, EBO;
  void setUpMesh();
};

#endif
