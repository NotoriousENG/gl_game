#pragma once

#include <components.hpp>
#include <flecs.h>
#include <glm/glm.hpp>
#include <plugins/plugin.hpp>

class Transform2DPlugin : public Plugin {
public:
  void addSystems(flecs::world &world) override;
};