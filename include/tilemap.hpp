#pragma once
#include "components.hpp"
#include "sprite-batch.hpp"
#include "texture.hpp"
#include "tmxlite/Map.hpp"
#include "tmxlite/TileLayer.hpp"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Tilemap {
public:
  std::vector<std::shared_ptr<Texture>> textures;
  Tilemap(const char *path);
  ~Tilemap();
  void Draw(SpriteBatch *spriteBatch);
  bool IsCollidingWith(SDL_Rect *other);

private:
  tmx::Map map;
};