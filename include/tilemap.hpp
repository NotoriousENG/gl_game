#pragma once
#include "sprite-batch.hpp"
#include "texture.hpp"
#include "tmxlite/Map.hpp"
#include "tmxlite/TileLayer.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Tilemap {
public:
  struct Subset final {
    unsigned vbo = 0;
    unsigned texture = 0;
    unsigned lookup = 0;
  };
  std::vector<Subset> subsets;
  std::vector<std::shared_ptr<Texture>> textures;
  Tilemap(const char *path);
  ~Tilemap();
  void Draw(SpriteBatch *spriteBatch);

private:
  tmx::Map map;
};