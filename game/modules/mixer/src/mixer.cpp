#include "mixer.hpp"

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

void Mixer::ToggleMute() {
  SDL_Log("Toggle Mute\n");
  if (!this->isMuted) {
    Mix_VolumeMusic(0);
    Mix_Volume(-1, 0);
    this->isMuted = true;
  } else {
    Mix_VolumeMusic(MIX_MAX_VOLUME);
    Mix_Volume(-1, MIX_MAX_VOLUME);
    this->isMuted = false;
  }
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

SoundEffect::SoundEffect(const char *path) {
  // loadWAV returns a Mix_Chunk pointer, it loads other formats too
  this->sdl_chunk = Mix_LoadWAV(path);

  if (this->sdl_chunk == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to load sound effect: %s\n",
                 Mix_GetError());
  }
}

SoundEffect::~SoundEffect() {
  if (this->sdl_chunk != nullptr) {
    Mix_FreeChunk(this->sdl_chunk);
    SDL_Log("Sound effect closed\n");
  }
}

void SoundEffect::play() {
  if (Mix_PlayChannel(-1, this->sdl_chunk, 0) == -1) {
    SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to play sound effect: %s\n",
                 Mix_GetError());
  }
}
