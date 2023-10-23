#include "spritesheet.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <utils.hpp>

SpriteSheet::SpriteSheet(const char *atlasPath, const char *texturePath) {
  this->texture = std::make_shared<Texture>(texturePath);
  this->spriteRects = this->loadAtlas(atlasPath);
}

SpriteSheet::~SpriteSheet() {}

Texture *SpriteSheet::GetTexture() { return this->texture.get(); }

const SDL_Rect SpriteSheet::GetSpriteRect(size_t index) {
  // validate the index
  if (index < 0 || index >= this->spriteRects.size()) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "SpriteSheet::GetSpriteRect: index out of range");
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "index: %d", index);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "spriteRects.size(): %d",
                 this->spriteRects.size());
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Returning spriteRects[0] instead");
    return this->spriteRects[0];
  }
  return this->spriteRects[index];
}

const glm::vec4 SpriteSheet::GetSpriteRectAsVec4(size_t index) {
  return sdlRectToVec4(this->GetSpriteRect(index));
}

const size_t SpriteSheet::GetSpriteCount() { return this->spriteRects.size(); }

// the atlas is json, an array of objects with x, y, w, h (integers)
// read into an vector of SDL_Rects
const std::vector<SDL_Rect> SpriteSheet::loadAtlas(const char *atlasPath) {
  std::vector<SDL_Rect> rects;
  std::ifstream atlasFile(atlasPath);
  nlohmann::json atlasJson;
  atlasFile >> atlasJson;
  for (auto &rect : atlasJson) {
    SDL_Rect r;
    r.x = rect["x"];
    r.y = rect["y"];
    r.w = rect["w"];
    r.h = rect["h"];
    rects.push_back(r);
  }
  return rects;
}
