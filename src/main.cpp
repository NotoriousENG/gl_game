#include "main.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include <glad/glad.h>

#include "defs.hpp"

static SDL_Window *window = NULL;
static SDL_GLContext glContext = NULL;
static SDL_Renderer *renderer = NULL;

static unsigned int framebuffer;
static unsigned int textureColorbuffer;
static unsigned int rbo;

void cleanup() {

  TTF_Quit();
  SDL_Log("TTF_Quit()");

  Mix_Quit();
  SDL_Log("Mix_Quit()");

  SDL_DestroyRenderer(renderer);
  SDL_Log("SDL_DestroyRenderer()");

  SDL_DestroyWindow(window);
  SDL_Log("SDL_DestroyWindow()");

  IMG_Quit();
  SDL_Log("IMG_Quit()");

  glDeleteTextures(1, &textureColorbuffer);
  SDL_Log("glDeleteTextures()");
  glDeleteRenderbuffers(1, &rbo);
  SDL_Log("glDeleteRenderbuffers()");
  glDeleteFramebuffers(1, &framebuffer);
  SDL_Log("glDeleteFramebuffers()");

  SDL_GL_DeleteContext(glContext);
  SDL_Log("SDL_GL_DeleteContext()");

  SDL_Quit();
  SDL_Log("SDL_Quit()");
}

int main(int argc, char *argv[]) {
  SDL_Log("Launching ...");
  atexit(cleanup);

  // rendererFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
  // prevent screen tear
  int rendererFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC |
                      SDL_RENDERER_TARGETTEXTURE;
  int windowFlags = SDL_WINDOW_OPENGL;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s",
                 SDL_GetError());
    exit(1);
  }

  // Initialize SDL_GL
  SDL_GL_LoadLibrary(NULL); // Default OpenGL is fine.

  // Request an OpenGL 4.5 context (should be core)
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL,
                      1); // Enable hardware or software rendering
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
  // Also request a stencil buffer
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  // Also request a depth buffer
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  // Request a debug context.
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

  // Enable MSAA
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  // Set Samples to 4
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

  window = SDL_CreateWindow(GAME_NAME, SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, SCALED_SCREEN_WIDTH,
                            SCALED_SCREEN_HEIGHT, windowFlags);

  glContext = SDL_GL_CreateContext(window);
  if (!glContext) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to create OpenGL context: %s", SDL_GetError());
    exit(1);
  }

  SDL_GL_SetSwapInterval(1); // Enable vsync

  // Check OpenGL properties
  SDL_Log("OpenGL loaded");
  gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
  SDL_Log("Vendor: %s", glGetString(GL_VENDOR));
  SDL_Log("Renderer: %s", glGetString(GL_RENDERER));
  SDL_Log("Version: %s", glGetString(GL_VERSION));

  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  // glDebugMessageCallback(openglCallbackFunction, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL,
                        GL_FALSE);
  glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE,
                        0, NULL, GL_TRUE);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  // glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.25f, .25f, 0.25f, 1.0f);

  // generate frame buffer
  glGenFramebuffers(1, &framebuffer);

  // generate texture
  glGenTextures(1, &textureColorbuffer);

  // create a renderbuffer object for depth and stencil attachment (we won't be
  // sampling these)
  glGenRenderbuffers(1, &rbo);

  // frequency of 44100 (CD quality), the default format, 2 channels (stereo)
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL Mixer");
    exit(1);
  }

  if (TTF_Init() < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL TTF");
    exit(1);
  }

  Mix_AllocateChannels(MAX_SOUND_CHANNELS);

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

  renderer =
      SDL_CreateRenderer(window, -1, rendererFlags); // -1 = default driver

  if (!renderer) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create renderer: %s",
                 SDL_GetError());
    exit(1);
  }

  if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL Image");
    exit(1);
  }

  // scale all rendering
  SDL_RenderSetScale(renderer, RENDER_SCALE, RENDER_SCALE);
  // set blend mode
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  bool running = true;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: {
        running = false;
      } break;
      }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glEnable(GL_DEPTH_TEST);                            // enable depth testing
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear framebuffer
    SDL_GL_SwapWindow(window);
  }

  return 0;
}