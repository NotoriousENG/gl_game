#pragma once

#include <components.hpp>
#include <flecs.h>
#include <glm/glm.hpp>
#include <plugins/physics.hpp>

// components
struct Enemy {
  bool seesPlayer;
  Enemy() : seesPlayer(false) {}
};

// systems
void enemyUpdate(Enemy *e, Velocity *v, Path *p, Transform2D *t);

// plugin
struct EnemyPlugin {
  void addSystems(flecs::world &world);
};