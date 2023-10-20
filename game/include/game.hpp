#pragma once

#include <memory>
#include <sprite-batch.hpp>


class Game {
public:
  Game();
  ~Game();
  int init();
  int update();
  int unload();
  int close();
  std::unique_ptr<SpriteBatch> spriteBatcher;
};