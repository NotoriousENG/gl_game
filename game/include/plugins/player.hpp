#pragma once

#include "components.hpp"
#include "flecs.h"
#include "input.hpp"
#include "plugin.hpp"
#include "plugins/graphics.hpp"
#include "plugins/physics.hpp"
#include <mixer.hpp>

// components:
struct Player {
  std::string name;
  bool isAttacking;                         // @TODO move to child collider
  std::shared_ptr<SoundEffect> soundEffect; // @TODO make own component
  std::shared_ptr<Music> music;             // @TODO make own component
  glm::vec4 defaultRect;                    // @TODO move this
};

// systems:
void playerUpdate(flecs::iter it, Player *p, Velocity *v, CollisionVolume *c,
                  AnimatedSprite *s, Transform2D *t, Groundable *g);

// plugin:
class PlayerPlugin : public Plugin {
public:
  void addSystems(flecs::world &ecs) override;
};