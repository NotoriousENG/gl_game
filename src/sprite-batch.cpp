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
}

void SpriteBatch::Draw(glm::vec4 destRect, glm::vec4 srcRect, glm::vec4 color,
                       Texture *texture, float rotation) {
  if (this->texture != texture) {
    this->Flush();
    this->texture = texture;
  }

  glm::vec2 center(destRect.x + destRect.z * 0.5f,
                   destRect.y + destRect.w * 0.5f);

  glm::vec2 topLeft(-destRect.z * 0.5f, -destRect.w * 0.5f);
  glm::vec2 topRight(destRect.z * 0.5f, -destRect.w * 0.5f);
  glm::vec2 bottomLeft(-destRect.z * 0.5f, destRect.w * 0.5f);
  glm::vec2 bottomRight(destRect.z * 0.5f, destRect.w * 0.5f);

  // Rotate the vertices
  glm::mat2 rotationMatrix(glm::cos(rotation), -glm::sin(rotation),
                           glm::sin(rotation), glm::cos(rotation));
  topLeft = rotationMatrix * topLeft + center;
  topRight = rotationMatrix * topRight + center;
  bottomLeft = rotationMatrix * bottomLeft + center;
  bottomRight = rotationMatrix * bottomRight + center;

  // Calculate the vertex index offset for the current sprite
  int vertexIndexOffset = this->vertices.size();

  // Add vertices to the list
  this->vertices.push_back(
      Vertex(topLeft, glm::vec2(srcRect.x, srcRect.y), color));
  this->vertices.push_back(
      Vertex(topRight, glm::vec2(srcRect.z, srcRect.y), color));
  this->vertices.push_back(
      Vertex(bottomLeft, glm::vec2(srcRect.x, srcRect.w), color));
  this->vertices.push_back(
      Vertex(bottomRight, glm::vec2(srcRect.z, srcRect.w), color));

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
