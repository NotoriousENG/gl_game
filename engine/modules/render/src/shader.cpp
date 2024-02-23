#include "shader.hpp"
#include <SDL.h>
#include <fstream>
#include <sstream>
#include <vector>

Shader::Shader() {}

Shader::~Shader() {
  if (shader != 0) {
    glDeleteShader(shader);
  }
}

bool Shader::LoadFromFile(const char *filePath, GLenum shaderType) {

  // Load vertex shader
  std::ifstream shaderFile(filePath);

  if (!shaderFile.good()) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open shader file: %s",
                 filePath);
    return false;
  }

  std::stringstream shaderStream;
  shaderStream << shaderFile.rdbuf();
  std::string shaderSource = shaderStream.str();

  return LoadFromString(shaderSource, shaderType);
}

bool Shader::LoadFromString(std::string source, GLenum shaderType) {
  // Create vertex shader object
  this->type = shaderType;
  this->shader = glCreateShader(shaderType);

  const char *shaderSourcePtr = source.c_str();
  glShaderSource(this->shader, 1, &shaderSourcePtr, nullptr);
  glCompileShader(this->shader);

  // Check vertex shader compilation status
  GLint isCompiled;
  glGetShaderiv(this->shader, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled != GL_TRUE) {
    // Handle shader compilation error
    GLint logLength;
    glGetShaderiv(this->shader, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<GLchar> logBuffer(logLength);
    glGetShaderInfoLog(this->shader, logLength, nullptr, logBuffer.data());
    std::string log(logBuffer.begin(), logBuffer.end());
    // Handle or log the shader compilation error
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to compile %i shader: %s", shaderType, log.c_str());
    glDeleteShader(this->shader);
    return false;
  }

  return true;
}

void Shader::AttatchToProgram(GLuint program) {
  if (this->shader != 0) {
    glAttachShader(program, this->shader);
    return;
  }
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
               "Failed to attach shader to program: shader is not initialized");
}
