#include "spritesheet.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <utils.hpp>

SpriteSheet::SpriteSheet(const char *atlasPath) { this->loadAtlas(atlasPath); }

SpriteSheet::~SpriteSheet() {}

Texture *SpriteSheet::GetTexture() { return this->texture.get(); }

const glm::vec4 SpriteSheet::GetAtlasRect(size_t index) {
  // validate the index
  if (index < 0 || index >= this->numRects) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "SpriteSheet::GetSpriteRect: index out of range");
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "index: %d", index);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "numRects: %d", this->numRects);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Returning spriteRects[0] instead");

    return glm::vec4(this->atlas[0], this->atlas[1], this->atlas[2],
                     this->atlas[3]);
  }
  return glm::vec4(this->atlas[index * 4], this->atlas[index * 4 + 1],
                   this->atlas[index * 4 + 2], this->atlas[index * 4 + 3]);
}

const size_t SpriteSheet::GetSpriteCount() { return this->numRects; }

SpriteAnimation *SpriteSheet::GetAnimation(const char *name) {
  if (this->animations.find(name) != this->animations.end()) {
    return &this->animations[name];
  }
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
               "SpriteSheet::GetAnimation: animation not found");
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "animation name: %s", name);
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Returning empty animation");
  return &this->animations["default"];
}

glm::vec4 SpriteSheet::GetAnimationRect(const SpriteAnimation *animation,
                                        size_t index) {
  return this->GetAtlasRect(animation->frames[index]);
}

void SpriteSheet::loadAtlas(const char *atlasPath) {
  std::vector<SDL_Rect> rects;
  std::ifstream atlasFile(atlasPath);
  nlohmann::json atlasJson;
  atlasFile >> atlasJson;

  // get the path (without file) from the atlas path
  const std::string atlasPathStr = atlasPath;
  const std::string atlasDir =
      atlasPathStr.substr(0, atlasPathStr.find_last_of("/"));

  // load the texture
  const std::string texturePath = atlasJson["texture"];
  const std::string textureFullPath = (atlasDir + "/" + texturePath).c_str();
  this->texture = std::make_shared<Texture>(textureFullPath.c_str());
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Texture loaded");

  // load the atlas into memory as a single vector of ints
  std::vector<int> temp = atlasJson["atlas"];
  this->atlas = temp;
  this->numRects = temp.size() / 4;

  // load the animations
  nlohmann::json animations = atlasJson["animations"];
  for (auto &animation : animations.items()) {
    const std::string name = animation.key();
    const std::vector<int> frames = animation.value()["frames"];
    float frameTime = animation.value()["frameTime"];

    // get the wh (dimensions) of the first frame

    const glm::vec4 rect = this->GetAtlasRect(frames[0]);
    const glm::vec2 dimensions = glm::vec2(rect.z, rect.w);

    this->animations[name] = SpriteAnimation(frames, frameTime, dimensions);
  }
  const glm::vec4 rect = this->GetAtlasRect(0);
  const glm::vec2 dimensions = glm::vec2(rect.z, rect.w);
  this->animations["default"] = SpriteAnimation({0}, 0.0f, dimensions);
}
