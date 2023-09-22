#pragma once
#include "texture.hpp"
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
};