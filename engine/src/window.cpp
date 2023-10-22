#include "window.hpp"

Window::Window(const char *title, int width, int height) {
  // Initialize SDL
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Log("SDL initialized");

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
