#pragma once

#include <SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#ifndef GAME_NAME
#define GAME_NAME "Add Game Name to CMakeLists.txt"
#endif

#define ASPECT_RATIO ((float)WINDOW_WIDTH / (float)WINDOW_HEIGHT)

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
