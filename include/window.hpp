#pragma once

#include <SDL.h>

class Window {
public:
  Window(const char *title, int width, int height);
  ~Window();

  SDL_Window *GetSDLWindow() const;
  int GetWidth() const;
  int GetHeight() const;

private:
  SDL_Window *window;
  int width;
  int height;
};
