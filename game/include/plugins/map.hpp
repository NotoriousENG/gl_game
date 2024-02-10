#pragma once

#include "plugins/physics.hpp"
#include "plugins/plugin.hpp"
#include <tilemap.hpp>

// components:
struct Map {
  std::shared_ptr<Tilemap> value;
};

// systems:
void collideWithMap(Tilemap *map, flecs::entity e, Transform2D &t,
                    CollisionVolume &c, Groundable &g);

void LoadLevel(flecs::world &ecs, std::shared_ptr<Tilemap> map);

void DrawColliders(flecs::world &ecs, SpriteBatch *sb);

// plugin:
class MapPlugin : public Plugin {
public:
  void addSystems(flecs::world &ecs) override;
};
