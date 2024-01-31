#include "plugins/transform.hpp"

void Transform2DPlugin::addSystems(flecs::world &world) {
  // update global positions for children
  world.system<Transform2D>().each([](flecs::entity e, Transform2D &t) {
    const auto parent = e.parent();
    if (parent) {
      const auto parent_t = parent.get<Transform2D>();
      t.global_position = parent_t->global_position;
      t.global_position.x += t.position.x;
      t.global_position.y += t.position.y;
    } else {
      t.global_position = t.position;
    }
  });
}
