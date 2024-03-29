#pragma once

#include "plugins/plugin.hpp"
#include <SDL.h>
#include <components.hpp>
#include <flecs.h>
// components:
struct Groundable {
  bool isGrounded;
};

struct CollisionVolume {
  glm::vec4 vertices;
};

struct CollisionEvent {
  flecs::entity entity1;
  flecs::entity entity2;
  Transform2D *transform1;
  Transform2D *transform2;
  CollisionVolume *collisionVolume1;
  CollisionVolume *collisionVolume2;
};

struct PhysicsBody {};

struct StaticBody {};

struct Velocity {
  glm::vec2 value;
};

struct Gravity {
  float value;
};

struct LiveFor {
  float seconds;
};

// utils:
void push_rect_transform(const SDL_Rect &rect, const SDL_Rect &pushedBy,
                         Transform2D &t1, CollisionVolume &c1);

// systems:
void applyGravity(flecs::iter it, Velocity *v, Groundable *g);

void applyVelocity(flecs::iter it, Velocity *v, Transform2D *t);

void runEntityCollisions(flecs::entity e1, Transform2D &t1, CollisionVolume &c1,
                         flecs::entity e2, Transform2D &t2,
                         CollisionVolume &c2);

void dieOfOldAge(flecs::iter it, LiveFor *l);

// plugin:
class PhysicsPlugin : public Plugin {
public:
  void addSystems(flecs::world &ecs) override;
};