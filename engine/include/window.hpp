#pragma once

#include <SDL.h>

class Window {
public:
  Window(const char *title, int width, int height);
  ~Window();

  SDL_Window *GetSDLWindow() const;

private:
  SDL_Window *window;
};
