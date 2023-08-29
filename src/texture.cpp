#include "texture.hpp"
#include <SDL.h>
#include <SDL_image.h>

Texture::Texture(const char *filename) {
  // Load image using SDL_image
  SDL_Surface *surface = IMG_Load(filename);
  if (!surface) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load texture: %s",
                 IMG_GetError());

    // Free the surface
    SDL_FreeSurface(surface);
    return;
  }

  glGenTextures(1, &this->texture);
  glBindTexture(GL_TEXTURE_2D, this->texture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, surface->pixels);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Unbind the texture
  glBindTexture(GL_TEXTURE_2D, 0);
  // Free the surfaces
  SDL_FreeSurface(surface);
}

Texture::~Texture() { glDeleteTextures(1, &this->texture); }

GLuint Texture::GetGLTexture() { return this->texture; }

glm::vec4 Texture::GetTextureRect() {
  glm::vec4 rect;
  glBindTexture(GL_TEXTURE_2D, this->texture);
  glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &rect.z);
  glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &rect.w);
  glBindTexture(GL_TEXTURE_2D, 0);
  return rect;
}
