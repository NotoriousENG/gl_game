#include "main.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "defs.hpp"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

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
  int windowFlags = 0;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s",
                 SDL_GetError());
    exit(1);
  }

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

  window = SDL_CreateWindow(GAME_NAME, SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, SCALED_SCREEN_WIDTH,
                            SCALED_SCREEN_HEIGHT, windowFlags);

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

    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);
  }

  return 0;
}