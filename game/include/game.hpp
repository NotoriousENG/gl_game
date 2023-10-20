#pragma once

#include <memory>
#include <sprite-batch.hpp>
#include <texture.hpp>

class Game {
public:
  Game();
  ~Game();
  int init();
  int update();
  int unload();
  int close();
  std::unique_ptr<SpriteBatch> spriteBatcher;
  std::shared_ptr<Texture> textureTink;
  std::shared_ptr<Texture> textureAnya;
};