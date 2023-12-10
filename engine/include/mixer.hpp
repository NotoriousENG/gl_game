#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include <memory>

class Mixer {
public:
  Mixer();
  ~Mixer();

  void ToggleMute();

private:
  bool isMuted = false;
};

class Music {
public:
  Music(const char *path);
  ~Music();

  void play_on_loop();

private:
  Mix_Music *sdl_music;
};

class SoundEffect {
public:
  SoundEffect(const char *path);
  ~SoundEffect();

  void play();

private:
  Mix_Chunk *sdl_chunk;
};