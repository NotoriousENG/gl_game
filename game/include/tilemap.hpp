#pragma once
#include "sprite-batch.hpp"
#include "texture.hpp"
#include "tmxlite/Map.hpp"
#include "tmxlite/TileLayer.hpp"
#include <SDL2/SDL.h>
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
  void DrawColliders(SpriteBatch *spriteBatch);
  void IsCollidingWith(SDL_Rect *other, SDL_Rect &found, uint64_t entity,
                       bool &isGrounded);

  std::vector<tmx::Object> GetObjects();
  tmx::Object GetObjectByHandle(const int handle);
  SDL_Rect GetBounds();

  bool HasCollision(uint64_t entity);

private:
  void initObjects();
  tmx::Map map;
  std::vector<tmx::Object> objects;
  std::set<uint64_t> entitiesCollidingWithMap;
};