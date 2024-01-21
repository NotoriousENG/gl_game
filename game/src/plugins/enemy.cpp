#include "plugins/enemy.hpp"

void enemyUpdate(Enemy &e, Velocity &v, Path &p, Transform2D &t) {
  // move towards target point if we are within distance 0.1f,
  // update the target point
  const auto targetPoint = p.points[p.targetPointIndex];
  const auto distance = fabs(glm::distance(t.position, targetPoint));
  const float speed = 100.0f;

  if (distance < 1.0f) {
    p.targetPointIndex++;
    if (p.targetPointIndex >= p.points.size()) {
      p.targetPointIndex = 0;
      return;
    }
  }

  const auto direction = glm::normalize(targetPoint - t.position);
  if (direction.x < 0) {
    t.scale.x = -1 * fabs(t.scale.x);
  } else {
    t.scale.x = fabs(t.scale.x);
  }
  v.value = direction * speed;
}

void EnemyPlugin::addSystems(flecs::world &world) {
  // enemy movement
  world.system<Enemy, Velocity, Path, Transform2D>().iter(
      ([](flecs::iter it, Enemy *e, Velocity *v, Path *p, Transform2D *t) {
        const auto dt = it.delta_time();

        for (int i : it) {
          enemyUpdate(e[i], v[i], p[i], t[i]);
        }
      }));
}
