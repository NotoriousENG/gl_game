#include "window.hpp"

Window::Window(const char *title, int width, int height) {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Failed to init SDL!\n");
    return;
  }

  // Create SDL window
  window =
      SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                       width, height, SDL_WINDOW_OPENGL);
  SDL_Log("SDL window created");
}

Window::~Window() {
  // Destroy SDL window
  SDL_DestroyWindow(window);
  SDL_Log("Window destroyed");

  // Quit SDL
  SDL_Quit();
  SDL_Log("SDL quit");
}

SDL_Window *Window::GetSDLWindow() const { return window; }
