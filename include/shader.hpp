#pragma once
#include <glad/glad.h>
#include <string>

class Shader {
public:
  Shader();
  ~Shader();

  bool LoadFromFile(const char *filePath, GLenum shaderType);
  bool LoadFromString(std::string source, GLenum shaderType);
  void AttatchToProgram(GLuint program);

private:
  GLuint shader;
  GLuint type;
};