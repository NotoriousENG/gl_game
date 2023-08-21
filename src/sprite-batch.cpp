#include "sprite-batch.hpp"
#include "defs.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

SpriteBatch::SpriteBatch(glm::vec2 windowSize) {
  this->vertexShader.LoadFromFile("assets/shaders/sprite.vert",
                                  GL_VERTEX_SHADER);
  this->fragmentShader.LoadFromFile("assets/shaders/sprite.frag",
                                    GL_FRAGMENT_SHADER);

  this->shaderProgram = glCreateProgram();

  this->vertexShader.AttatchToProgram(this->shaderProgram);
  this->fragmentShader.AttatchToProgram(this->shaderProgram);

  glLinkProgram(this->shaderProgram);

  GLint linkStatus;

  glGetProgramiv(this->shaderProgram, GL_LINK_STATUS, &linkStatus);

  if (linkStatus != GL_TRUE) {
    GLint logLength;
    glGetProgramiv(this->shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<GLchar> logBuffer(logLength);
    glGetProgramInfoLog(this->shaderProgram, logLength, nullptr,
                        logBuffer.data());
    std::string log(logBuffer.begin(), logBuffer.end());
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to link shader program: %s", log.c_str());
    glDeleteProgram(this->shaderProgram);
    return;
  }

  this->textureUniform = glGetUniformLocation(this->shaderProgram, "texture");
  this->texture = nullptr;

  glGenBuffers(1, &this->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid *)sizeof(glm::vec2));
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid *)sizeof(glm::vec4));
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  this->screenTransformUniform =
      glGetUniformLocation(this->shaderProgram, "screenTransform");
  this->SetScreenSize(windowSize);
}

SpriteBatch::~SpriteBatch() { glDeleteProgram(this->shaderProgram); }

void SpriteBatch::Draw(glm::vec4 destRect, glm::vec4 srcRect, glm::vec4 color,
                       Texture *texture) {
  if (this->texture != texture) {
    this->Flush();

    // @TODO memory manage texture

    this->texture = texture;
  }

  this->vertices.push_back(Vertex(glm::vec2(destRect.x, destRect.y),
                                  glm::vec2(srcRect.x, srcRect.y), color));
  this->vertices.push_back(
      Vertex(glm::vec2(destRect.x + destRect.z, destRect.y),
             glm::vec2(srcRect.z, srcRect.y), color));
  this->vertices.push_back(
      Vertex(glm::vec2(destRect.x, destRect.y + destRect.w),
             glm::vec2(srcRect.x, srcRect.w), color));
  this->vertices.push_back(
      Vertex(glm::vec2(destRect.x + destRect.z, destRect.y),
             glm::vec2(srcRect.z, srcRect.y), color));
  this->vertices.push_back(
      Vertex(glm::vec2(destRect.x, destRect.y + destRect.w),
             glm::vec2(srcRect.x, srcRect.w), color));
  this->vertices.push_back(
      Vertex(glm::vec2(destRect.x + destRect.z, destRect.y + destRect.w),
             glm::vec2(srcRect.z, srcRect.w), color));
}

void SpriteBatch::Flush() {
  if (this->vertices.size() == 0 || this->texture == nullptr) {
    return;
  }

  glUseProgram(this->shaderProgram);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, this->texture->GetGLTexture());
  glUniform1i(this->textureUniform, 0);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex),
               &this->vertices[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glUniformMatrix3fv(this->screenTransformUniform, 1, GL_FALSE,
                     glm::value_ptr(this->screenTransform));

  glEnableVertexAttribArray(0); // position
  glEnableVertexAttribArray(1); // uv
  glEnableVertexAttribArray(2); // color

  glDrawArrays(GL_TRIANGLES, 0, this->vertices.size());

  glDisableVertexAttribArray(0); // position
  glDisableVertexAttribArray(1); // uv
  glDisableVertexAttribArray(2); // color

  this->vertices.clear();
}

void SpriteBatch::SetScreenSize(glm::vec2 windowSize) {
  // openGL is [-1 to 1] on both axis, map from [0 to windowSize] to [-1 to 1]
  this->screenTransform[0][0] = 2 / windowSize.x;
  this->screenTransform[1][1] = 2 / windowSize.y;
  this->screenTransform[2][0] = -1;
  this->screenTransform[2][1] = -1;
}
