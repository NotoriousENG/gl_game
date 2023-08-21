#pragma once
#include <glad/glad.h>

class Texture {
public:
  Texture(const char *filename);
  ~Texture();

  GLuint GetGLTexture();

private:
  GLuint texture;
};