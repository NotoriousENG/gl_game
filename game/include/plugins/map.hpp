#pragma once

#include "plugins/physics.hpp"
#include "plugins/plugin.hpp"
#include <tilemap.hpp>

// components:
struct Map {
  Tilemap *value;
};

// systems:
void collideWithMap(Tilemap *map, flecs::entity e, Transform2D &t,
                    CollisionVolume &c, Groundable &g);

void LoadLevel(flecs::world &ecs, Tilemap *map);

// plugin:
class MapPlugin : public Plugin {
public:
  void addSystems(flecs::world &ecs) override;
};
