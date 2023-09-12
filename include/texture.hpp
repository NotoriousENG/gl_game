#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

class Texture {
public:
  Texture(const char *filename);
  ~Texture();

  GLuint GetGLTexture();

  glm::ivec4 GetTextureRect();

private:
  GLuint texture;
  int w;
  int h;
};