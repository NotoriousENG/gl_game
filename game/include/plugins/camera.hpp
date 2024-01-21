#pragma once

#include "plugins/plugin.hpp"
#include <components.hpp>
#include <flecs.h>

// components
struct Camera {
  glm::vec2 position;
};

// plugin
class CameraPlugin : public Plugin {
public:
  void addSystems(flecs::world &world) override;
};