#include "renderer.hpp"
#include <fstream>
#include <sstream>

#include "window.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "defs.hpp"

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
// Set the OpenGL context attributes (needed for renderdoc)
#ifndef EMSCRIPTEN
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                      SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

  this->glContext = SDL_GL_CreateContext(window->GetSDLWindow());
  if (!this->glContext) {
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
  glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE,
                        0, NULL, GL_TRUE);
#endif

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Set up OpenGL state
  glClearColor(0.25f, .25f, 0.25f, 1.0f);

  SDL_Log("OpenGL state initialized");
}

Renderer::~Renderer() {
  // Destroy OpenGL context
  SDL_GL_DeleteContext(this->glContext);
  SDL_Log("OpenGL context destroyed");
}

void Renderer::Clear() {
  // Clear the color buffer
  glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Present() {
  // Swap the front and back buffers
  SDL_GL_SwapWindow(SDL_GL_GetCurrentWindow());
}

void Renderer::SetClearColor(float r, float g, float b, float a) {
  glClearColor(r, g, b, a);
}
