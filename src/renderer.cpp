#include "renderer.hpp"
#include <fstream>
#include <sstream>

#include "window.hpp"

#include <glm/gtc/type_ptr.hpp>

void Renderer::openglCallbackFunction(GLenum source, GLenum type, GLuint id,
                                      GLenum severity, GLsizei length,
                                      const GLchar *message,
                                      const void *userParam) {
  (void)source;
  (void)type;
  (void)id;
  (void)severity;
  (void)length;
  (void)userParam;
  fprintf(stderr, "%s\n", message);
  if (severity == GL_DEBUG_SEVERITY_HIGH) {
    fprintf(stderr, "Aborting...\n");
    abort();
  }
}

Renderer::Renderer(Window *window) {
  // Create OpenGL context
  glContext = SDL_GL_CreateContext(window->GetSDLWindow());
  if (!glContext) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to create OpenGL context: %s", SDL_GetError());
    return;
  }

  // Initialize GLAD
  gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress);
  SDL_Log("OpenGL %d.%d", GLVersion.major, GLVersion.minor);
  SDL_Log("OpenGL %s, GLSL %s", glGetString(GL_VERSION),
          glGetString(GL_SHADING_LANGUAGE_VERSION));
  SDL_Log("Vendor: %s", glGetString(GL_VENDOR));
  SDL_Log("Renderer: %s", glGetString(GL_RENDERER));

  // Enable the debug callback
  #ifndef EMSCRIPTEN
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(openglCallbackFunction, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL,
                          GL_FALSE);
    glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR,
                          GL_DONT_CARE, 0, NULL, GL_TRUE);
  #endif

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Set up OpenGL state
  glClearColor(0.25f, .25f, 0.25f, 1.0f);

  SDL_Log("OpenGL state initialized");

  // Init Quad
 {
    glGenVertexArrays(1, &this->quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
                 GL_STATIC_DRAW);

    glBindVertexArray(quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  SDL_Log("Renderer created");
  // Create shader program
  CreateShaderProgram();

  SDL_Log("Shader program created");
}

Renderer::~Renderer() {
  // Destroy OpenGL context
  SDL_GL_DeleteContext(glContext);
}

void Renderer::Clear() {
  // Clear the color buffer
  glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Present() {
  // Swap the front and back buffers
  SDL_GL_SwapWindow(SDL_GL_GetCurrentWindow());
}

void Renderer::RenderSprite(const Sprite &sprite) {
  // Activate the shader program
  glUseProgram(shaderProgram);

  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::mat4(1.0f);
  glm::vec3 spriteColor = glm::vec3(1.0f);

  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE,
                     glm::value_ptr(model));

  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE,
                     glm::value_ptr(view));

  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1,
                     GL_FALSE, glm::value_ptr(projection));

  glUniform3fv(glGetUniformLocation(shaderProgram, "spriteColor"), 1,
               glm::value_ptr(spriteColor));

  glActiveTexture(GL_TEXTURE0);
  // Bind the texture
  glBindTexture(GL_TEXTURE_2D, sprite.texture);

  // Bind the vertex array
  glBindVertexArray(quadVAO);

  // Draw the sprite
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindVertexArray(0);
}



GLuint Renderer::LoadTexture(const std::string &filepath) {
  // Load image using SDL_image
  SDL_Surface *surface = IMG_Load(filepath.c_str());
  if (!surface) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load texture: %s",
                 IMG_GetError());
    return 0;
  }

  // Create OpenGL texture
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Load image data into texture
  GLenum format = (surface->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;
  glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h, 0, format,
               GL_UNSIGNED_BYTE, surface->pixels);

  // Free the surface
  SDL_FreeSurface(surface);

  return textureID;
}

void Renderer::CreateShaderProgram() {
  // Create shader program
  shaderProgram = glCreateProgram();

  // Load vertex shader
  std::ifstream vertexShaderFile("assets/shaders/sprite.vert");
  std::stringstream vertexShaderStream;
  vertexShaderStream << vertexShaderFile.rdbuf();
  std::string vertexShaderSource = vertexShaderStream.str();
  const char *vertexShaderSourcePtr = vertexShaderSource.c_str();

  // Create vertex shader object
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSourcePtr, nullptr);
  glCompileShader(vertexShader);

  // Check vertex shader compilation status
  GLint vertexShaderCompileStatus;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexShaderCompileStatus);
  if (vertexShaderCompileStatus != GL_TRUE) {
    // Handle shader compilation error
    GLint logLength;
    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<GLchar> logBuffer(logLength);
    glGetShaderInfoLog(vertexShader, logLength, nullptr, logBuffer.data());
    std::string log(logBuffer.begin(), logBuffer.end());
    // Handle or log the shader compilation error
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to compile vertex shader: %s", log.c_str());
    glDeleteShader(vertexShader);
    return;
  }

  // Load fragment shader
  std::ifstream fragmentShaderFile("assets/shaders/sprite.frag");
  std::stringstream fragmentShaderStream;
  fragmentShaderStream << fragmentShaderFile.rdbuf();
  std::string fragmentShaderSource = fragmentShaderStream.str();
  const char *fragmentShaderSourcePtr = fragmentShaderSource.c_str();

  // Create fragment shader object
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSourcePtr, nullptr);
  glCompileShader(fragmentShader);

  // Check fragment shader compilation status
  GLint fragmentShaderCompileStatus;
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS,
                &fragmentShaderCompileStatus);
  if (fragmentShaderCompileStatus != GL_TRUE) {
    // Handle shader compilation error
    GLint logLength;
    glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<GLchar> logBuffer(logLength);
    glGetShaderInfoLog(fragmentShader, logLength, nullptr, logBuffer.data());
    std::string log(logBuffer.begin(), logBuffer.end());
    // Handle or log the shader compilation error
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to compile fragment shader: %s", log.c_str());
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return;
  }

  // Attach shaders to the program
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);

  // Link the program
  glLinkProgram(shaderProgram);

  // Check program linking status
  GLint programLinkStatus;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &programLinkStatus);
  if (programLinkStatus != GL_TRUE) {
    // Handle program linking error
    GLint logLength;
    glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<GLchar> logBuffer(logLength);
    glGetProgramInfoLog(shaderProgram, logLength, nullptr, logBuffer.data());
    std::string log(logBuffer.begin(), logBuffer.end());
    // Handle or log the program linking error
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to link shader program: %s", log.c_str());
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(shaderProgram);
    return;
  }

  // Clean up shader objects
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}