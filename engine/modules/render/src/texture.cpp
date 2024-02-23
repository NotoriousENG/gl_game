#include "texture.hpp"
#include <SDL.h>

#ifdef EMSCRIPTEN
#include <SDL_image.h> // stb_image ahould be supported in emscripten, not sure why it's not working

Texture::Texture(const char *filename) {
  // Load image using SDL_image
  SDL_Log("Loading texture: %s", filename);
  SDL_Surface *surface = IMG_Load(filename);
  this->w = surface->w;
  this->h = surface->h;
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

#endif
#ifndef EMSCRIPTEN
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const char *filename) {
  // Load image using stb_image
  int w, h, channels;
  unsigned char *image = stbi_load(filename, &w, &h, &channels, STBI_rgb_alpha);
  if (!image) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load texture: %s",
                 stbi_failure_reason());
    return;
  }
  this->w = w;
  this->h = h;

  glGenTextures(1, &this->texture);
  glBindTexture(GL_TEXTURE_2D, this->texture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               image);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Unbind the texture
  glBindTexture(GL_TEXTURE_2D, 0);
  // Free the image data
  stbi_image_free(image);
}

#endif

Texture::~Texture() { glDeleteTextures(1, &this->texture); }

GLuint Texture::GetGLTexture() { return this->texture; }

glm::ivec4 Texture::GetTextureRect() {
  return glm::ivec4(0, 0, this->w, this->h);
}
