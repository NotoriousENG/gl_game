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

  // Create and bind a VAO
  glGenVertexArrays(1, &this->vao);
  glBindVertexArray(this->vao);

  // Create and bind the EBO
  glGenBuffers(1, &this->ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);

  // Create and bind the VBO
  glGenBuffers(1, &this->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

  // Configure vertex attribute pointers in the VAO

  glEnableVertexAttribArray(0); // position
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)0);

  glEnableVertexAttribArray(1); // uv
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid *)sizeof(glm::vec2));

  glEnableVertexAttribArray(2); // color
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid *)sizeof(glm::vec4));

  glEnableVertexAttribArray(3); // rotation
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid *)(2 * sizeof(glm::vec2) + sizeof(glm::vec4)));

  // Unbind the VAO and VBO
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  this->screenTransformUniform =
      glGetUniformLocation(this->shaderProgram, "screenTransform");
  this->SetScreenSize(windowSize);
}

SpriteBatch::~SpriteBatch() {
  glDeleteBuffers(1, &this->vbo);
  glDeleteBuffers(1, &this->ebo);
  glDeleteVertexArrays(1, &this->vao);

  glDeleteProgram(this->shaderProgram);
}

void SpriteBatch::Draw(Texture *texture, glm::vec2 position, glm::vec2 scale,
                       float rotation, glm::vec4 color, glm::vec4 srcRect) {

  const glm::vec4 textureRect = texture->GetTextureRect();

  if (this->texture != texture) {
    this->Flush();
    this->texture = texture;
  }

  if (srcRect == glm::vec4(0, 0, 0, 0)) {
    srcRect = textureRect;
  }

  glm::vec2 center(position.x + (srcRect.z * scale.x) * 0.5f,
                   position.y + (srcRect.w * scale.y) * 0.5f);

  glm::vec2 scaledTopLeft(-srcRect.z * scale.x * 0.5f,
                          -srcRect.w * scale.y * 0.5f);
  glm::vec2 scaledTopRight(srcRect.z * scale.x * 0.5f,
                           -srcRect.w * scale.y * 0.5f);
  glm::vec2 scaledBottomLeft(-srcRect.z * scale.x * 0.5f,
                             srcRect.w * scale.y * 0.5f);
  glm::vec2 scaledBottomRight(srcRect.z * scale.x * 0.5f,
                              srcRect.w * scale.y * 0.5f);

  // Rotate the vertices
  const glm::mat2 rotationMatrix(glm::cos(rotation), -glm::sin(rotation),
                                 glm::sin(rotation), glm::cos(rotation));
  scaledTopLeft = rotationMatrix * scaledTopLeft + center;
  scaledTopRight = rotationMatrix * scaledTopRight + center;
  scaledBottomLeft = rotationMatrix * scaledBottomLeft + center;
  scaledBottomRight = rotationMatrix * scaledBottomRight + center;

  const float textureWidth = textureRect.z;
  const float textureHeight = textureRect.w;

  const glm::vec2 uvTopLeft(srcRect.x / textureWidth,
                            srcRect.y / textureHeight);
  const glm::vec2 uvTopRight((srcRect.x + srcRect.z) / textureWidth,
                             srcRect.y / textureHeight);
  const glm::vec2 uvBottomLeft(srcRect.x / textureWidth,
                               (srcRect.y + srcRect.w) / textureHeight);
  const glm::vec2 uvBottomRight((srcRect.x + srcRect.z) / textureWidth,
                                (srcRect.y + srcRect.w) / textureHeight);

  // Calculate the vertex index offset for the current sprite
  const int vertexIndexOffset = this->vertices.size();

  // Add vertices to the list
  this->vertices.push_back(Vertex(scaledTopLeft, uvTopLeft, color));
  this->vertices.push_back(Vertex(scaledTopRight, uvTopRight, color));
  this->vertices.push_back(Vertex(scaledBottomLeft, uvBottomLeft, color));
  this->vertices.push_back(Vertex(scaledBottomRight, uvBottomRight, color));

  // Add indices for the two triangles forming the quad for the current sprite
  // Add indices offset by vertexIndexOffset
  this->indices.push_back(vertexIndexOffset + 0);
  this->indices.push_back(vertexIndexOffset + 1);
  this->indices.push_back(vertexIndexOffset + 2);
  this->indices.push_back(vertexIndexOffset + 2);
  this->indices.push_back(vertexIndexOffset + 1);
  this->indices.push_back(vertexIndexOffset + 3);
}

void SpriteBatch::Flush() {
  if (this->vertices.size() == 0 || this->texture == nullptr) {
    return;
  }

  glUseProgram(this->shaderProgram);
  glBindVertexArray(this->vao); // Bind the VAO

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, this->texture->GetGLTexture());
  glUniform1i(this->textureUniform, 0);

  // Upload the vertex data to the GPU
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex),
               &this->vertices[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Upload the index data to the GPU
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint),
               &this->indices[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glUniformMatrix3fv(this->screenTransformUniform, 1, GL_FALSE,
                     glm::value_ptr(this->screenTransform));

  glEnableVertexAttribArray(0); // position
  glEnableVertexAttribArray(1); // uv
  glEnableVertexAttribArray(2); // color
  glEnableVertexAttribArray(3); // rotation

  glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);

  glDisableVertexAttribArray(0); // position
  glDisableVertexAttribArray(1); // uv
  glDisableVertexAttribArray(2); // color
  glDisableVertexAttribArray(3); // rotation

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind the EBO
  glBindVertexArray(0);                     // Unbind the VAO
  this->vertices.clear();
  this->indices.clear();
}

void SpriteBatch::SetScreenSize(glm::vec2 windowSize) {
  // the top left should be 0,0 and the bottom right should be windowSize.x,
  // windowSize.y
  this->screenTransform[0][0] = 2.0f / windowSize.x;
  this->screenTransform[1][1] = -2.0f / windowSize.y; // Negative to flip Y-axis
  this->screenTransform[2][0] = -1.0f;
  this->screenTransform[2][1] = 1.0f; // Adjusted to keep (0,0) at top-left
}
