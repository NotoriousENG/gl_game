#include "shader.hpp"
#include "texture.hpp"

#include <SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

struct Vertex {
  glm::vec2 position;
  glm::vec2 texCoords;
  glm::vec4 color;

  Vertex(glm::vec2 position, glm::vec2 texCoords, glm::vec4 color)
      : position(position), texCoords(texCoords), color(color) {}
};

class SpriteBatch {
public:
  SpriteBatch(glm::vec2 windowSize);
  ~SpriteBatch();

  void Draw(glm::vec4 destRect, glm::vec4 srcRect, glm::vec4 color,
            Texture *texture);
  void Flush();

  void SetScreenSize(glm::vec2 windowSize);

private:
  std::vector<Vertex> vertices;
  GLuint vbo;

  Shader vertexShader;
  Shader fragmentShader;

  GLuint shaderProgram;

  Texture *texture;
  GLuint textureUniform;

  glm::mat3 screenTransform;
  GLuint screenTransformUniform;
};
