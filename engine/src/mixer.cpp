#include "mixer.hpp"
#include "defs.hpp"

Mixer::Mixer() {
  // frequency of 44100 (CD quality), the default format, 2 channels (stereo)
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
    SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Couldn't initialize SDL Mixer\n");
    return;
  }
  Mix_AllocateChannels(MAX_SOUND_CHANNELS);
}

Mixer::~Mixer() {
  Mix_CloseAudio();
  Mix_Quit();
  SDL_Log("Mixer closed\n");
}

Music::Music(const char *path) {
  this->sdl_music = Mix_LoadMUS(path);

  if (this->sdl_music == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to load music: %s\n",
                 Mix_GetError());
  }
}

Music::~Music() {
  if (this->sdl_music != nullptr) {
    Mix_FreeMusic(this->sdl_music);
    SDL_Log("Music closed\n");
  }
}

void Music::play_on_loop() {
  if (Mix_PlayMusic(this->sdl_music, -1) == -1) {
    SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to play music: %s\n",
                 Mix_GetError());
  }
}