#pragma once

#include "flecs.h"
#include <components.hpp>
#include <font.hpp>
#include <memory>
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

  void push_rect_transform(const SDL_Rect &rect, const SDL_Rect &pushedBy,
                           Transform2D &t1, CollisionVolume &c1);

  std::unique_ptr<SpriteBatch> spriteBatcher;
  std::shared_ptr<Texture> textureTink;
  std::shared_ptr<Texture> textureAnya;

  std::shared_ptr<SpriteSheet> spritesheet;

  std::unique_ptr<Tilemap> tilemap;
  flecs::world world;

  std::unique_ptr<Font> font;

  bool drawColliders = false;
};