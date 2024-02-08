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
  std::shared_ptr<Texture> textureAnya;
  std::shared_ptr<Texture> textureArrow;
  std::shared_ptr<Texture> textureBall;

  std::shared_ptr<SpriteSheet> spritesheet;

  std::unique_ptr<Tilemap> tilemap;
  std::unique_ptr<Tilemap> tilemap2;
  flecs::world world;

  bool level1 = true;

  std::unique_ptr<Font> fontL;
  std::unique_ptr<Font> fontS;
  std::unique_ptr<Mixer> mixer;
  std::unique_ptr<Music> music;
  std::unique_ptr<SoundEffect> soundEffect;

  bool drawColliders = false;
};