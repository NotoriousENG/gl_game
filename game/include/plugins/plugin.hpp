#pragma once
#include "flecs.h"

class Plugin {
public:
  virtual void addSystems(flecs::world &ecs) = 0;
};