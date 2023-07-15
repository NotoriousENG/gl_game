#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Window;

struct Sprite {
  GLuint texture;
  glm::vec2 position;
  glm::vec2 size;
};

struct Vertex {
  glm::vec2 position;
  glm::vec2 texCoords;
};

class Renderer {
public:
  Renderer(Window *window);
  ~Renderer();

  void Clear();
  void Present();
  void RenderSprite(const Sprite &sprite);

  GLuint LoadTexture(const std::string &filepath);

private:
  SDL_GLContext glContext;
  GLuint vertexArray;
  GLuint vertexBuffer;
  GLuint shaderProgram;
  std::vector<Sprite> sprites;

  void CreateBuffers();
  void CreateShaderProgram();

  static void APIENTRY openglCallbackFunction(GLenum source, GLenum type,
                                              GLuint id, GLenum severity,
                                              GLsizei length,
                                              const GLchar *message,
                                              const void *userParam);

  // configure VAO/VBO
  unsigned int quadVAO;
  unsigned int quadVBO;
  const float quadVertices[24] = {
      // pos      // tex
      -.5f, -.5f, 1.0f, 1.0f, .5f, -.5f, 0.0f, 1.0f, -.5f, .5f,  1.0f, 0.0f,

      -.5f, .5f,  1.0f, 0.0f, .5f, .5f,  0.0f, 0.0f, .5f,  -.5f, 0.0f, 1.0f};
};
