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

  // Flip the image vertically (OpenGL expects the 0th row of pixels to be at
  // the bottom of the texture)
  SDL_Surface *flippedSurface = SDL_CreateRGBSurface(
      0, surface->w, surface->h, surface->format->BitsPerPixel,
      surface->format->Rmask, surface->format->Gmask, surface->format->Bmask,
      surface->format->Amask);

  for (int y = 0; y < surface->h; y++) {
    memcpy((Uint8 *)flippedSurface->pixels + y * flippedSurface->pitch,
           (Uint8 *)surface->pixels + (surface->h - y - 1) * surface->pitch,
           surface->pitch);
  }

  glGenTextures(1, &this->texture);
  glBindTexture(GL_TEXTURE_2D, this->texture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, flippedSurface->w, flippedSurface->h,
               0, GL_RGBA, GL_UNSIGNED_BYTE, flippedSurface->pixels);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Unbind the texture
  glBindTexture(GL_TEXTURE_2D, 0);
  // Free the surfaces
  SDL_FreeSurface(surface);
  SDL_FreeSurface(flippedSurface);
}

Texture::~Texture() { glDeleteTextures(1, &this->texture); }

GLuint Texture::GetGLTexture() { return this->texture; }
