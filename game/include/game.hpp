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

struct CollisionEvent {
  flecs::entity entity1;
  flecs::entity entity2;
  Transform2D *transform1;
  Transform2D *transform2;
  CollisionVolume *collisionVolume1;
  CollisionVolume *collisionVolume2;
};

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
  std::shared_ptr<Texture> textureAnya;

  std::shared_ptr<SpriteSheet> spritesheet;

  std::unique_ptr<Tilemap> tilemap;
  flecs::world world;

  std::unique_ptr<Font> fontL;
  std::unique_ptr<Font> fontS;
  std::unique_ptr<Mixer> mixer;
  std::unique_ptr<Music> music;
  std::unique_ptr<SoundEffect> soundEffect;

  bool drawColliders = false;
};