#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include <memory>

class Music {
public:
  Music(const char *path);
  ~Music();

  void play_on_loop();

private:
  Mix_Music *sdl_music;
};

class Mixer {
public:
  Mixer();
  ~Mixer();
};
