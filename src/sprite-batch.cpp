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

  this->textureUniform =
      glGetUniformLocation(this->shaderProgram, "albedoTexture");
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

  this->projectionUniform =
      glGetUniformLocation(this->shaderProgram, "projection");
  this->viewUniform = glGetUniformLocation(this->shaderProgram, "view");

  this->SetProjection(windowSize);
}

SpriteBatch::~SpriteBatch() {
  glDeleteBuffers(1, &this->vbo);
  glDeleteBuffers(1, &this->ebo);
  glDeleteVertexArrays(1, &this->vao);

  glDeleteProgram(this->shaderProgram);
}

void SpriteBatch::UpdateCamera(glm::vec2 position) {
  // update the view, so that the target is always in the center of the screen
  this->cameraPosition = position;
  this->view = glm::translate(
      glm::mat4(1.0f), glm::vec3(-position.x + this->windowSize.x / 2,
                                 -position.y + this->windowSize.y / 2, 0.0f));
}

void SpriteBatch::Draw(Texture *texture, glm::vec2 position, glm::vec2 scale,
                       float rotation, glm::vec4 color, glm::vec4 srcRect) {

  if (this->texture != texture) {
    this->Flush();
    this->texture = texture;
  }

  const glm::ivec4 textureRect =
      texture != nullptr ? texture->GetTextureRect() : glm::ivec4(0, 0, 1, 1);
  if (srcRect == glm::vec4(0, 0, 0, 0)) {
    srcRect = textureRect;
  }

  // Sprite culling
  // don't draw if outside of the camera rect bounds
  const auto bounds =
      this->windowSize /*- glm::vec2(128, 128)*/; // uncomment this to see the
                                                  // culling
  SDL_Rect cameraRectSDL = {
      static_cast<int>(static_cast<int>(this->cameraPosition.x) - bounds.x / 2),
      static_cast<int>(static_cast<int>(this->cameraPosition.y) - bounds.y / 2),
      static_cast<int>(bounds.x), static_cast<int>(bounds.y)};
  // get the bounding rect of the sprite (@TODO: account for rotation)
  SDL_Rect spriteRectSDL = {static_cast<int>(position.x),
                            static_cast<int>(position.y),
                            static_cast<int>(srcRect.z * scale.x),
                            static_cast<int>(srcRect.w * scale.y)};
  // check if the sprite is outside the camera rect
  if (!SDL_HasIntersection(&cameraRectSDL, &spriteRectSDL)) {
    return;
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
  if (this->vertices.size() == 0) {
    return;
  }

  glUseProgram(this->shaderProgram);
  glBindVertexArray(this->vao); // Bind the VAO

  GLuint whiteTexture;

  if (this->texture == nullptr) {
    // If the texture is undefined, bind a 1x1 white texture
    glGenTextures(1, &whiteTexture);
    glBindTexture(GL_TEXTURE_2D, whiteTexture);
    unsigned char whitePixel[] = {255, 255, 255, 255}; // White color
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 whitePixel);
    glUniform1i(this->textureUniform, 0);
  } else {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture->GetGLTexture());
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glUniform1i(this->textureUniform, 0);
  }

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

  glUniformMatrix4fv(this->projectionUniform, 1, GL_FALSE,
                     glm::value_ptr(this->projection));

  glUniformMatrix4fv(this->viewUniform, 1, GL_FALSE,
                     glm::value_ptr(this->view));

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
  if (this->texture == nullptr) {
    // Delete the temporary white texture if it was created
    glDeleteTextures(1, &whiteTexture);
  }
}

void SpriteBatch::SetProjection(glm::vec2 windowSize) {
  // create a projection matrix that will make the screen coordinates (0,0)
  // top left to (windowSize.x, windowSize.y) bottom right
  this->windowSize = windowSize;
  this->projection =
      glm::ortho(0.0f, this->windowSize.x, this->windowSize.y, 0.0f);
}
