#pragma once
#include "components.hpp"
#include "sprite-batch.hpp"
#include "texture.hpp"
#include "tmxlite/Map.hpp"
#include "tmxlite/TileLayer.hpp"
#include <SDL2/SDL.h>
#include <flecs.h>
#include <glm/glm.hpp>
#include <memory>
#include <set>
#include <vector>

class Tilemap {
public:
  std::vector<std::shared_ptr<Texture>> textures;
  Tilemap(const char *path);
  ~Tilemap();
  void Draw(SpriteBatch *spriteBatch);
  void IsCollidingWith(SDL_Rect *other, SDL_Rect &found, flecs::entity entity,
                       bool &isGrounded);
  SDL_Rect GetBounds();

  bool HasCollision(flecs::entity entity);

private:
  tmx::Map map;
  std::set<flecs::entity> entitiesCollidingWithMap;
};