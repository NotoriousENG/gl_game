#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Window;

class Renderer {
public:
  Renderer(Window *window);
  ~Renderer();

  void Clear();
  void Present();

private:
  SDL_GLContext glContext;

  static void APIENTRY openglCallbackFunction(GLenum source, GLenum type,
                                              GLuint id, GLenum severity,
                                              GLsizei length,
                                              const GLchar *message,
                                              const void *userParam);
};
