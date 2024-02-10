#pragma once

#include "flecs.h"
#include <components.hpp>
#include <font.hpp>
#include <memory>
#include <mixer.hpp>
#include <shared-data.hpp>
#include <sprite-batch.hpp>
#include <spritesheet.hpp>
#include <texture.hpp>
#include <tilemap.hpp>

class Game {
public:
  Game();
  ~Game();
  int init(SharedData *shared_data);
  int update();
  int unload();
  int close();

  std::unique_ptr<SpriteBatch> spriteBatcher;
  flecs::world world;

  bool level1 = true;

  std::unique_ptr<Mixer> mixer;

  bool drawColliders = false;
};