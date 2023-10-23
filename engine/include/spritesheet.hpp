#pragma once

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "texture.hpp"

class SpriteSheet {
public:
  SpriteSheet(const char *atlasPath, const char *texturePath);
  ~SpriteSheet();

  Texture *GetTexture();

  const SDL_Rect GetSpriteRect(size_t index);

  const glm::vec4 GetSpriteRectAsVec4(size_t index);

  const size_t GetSpriteCount();

private:
  const std::vector<SDL_Rect> loadAtlas(const char *atlasPath);

  std::shared_ptr<Texture> texture;
  std::vector<SDL_Rect> spriteRects;
};
