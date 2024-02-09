#include "plugins/map.hpp"
#include "plugins/physics.hpp"
#include <memory>

void collideWithMap(Tilemap *map, flecs::entity e, Transform2D &t,
                    CollisionVolume &c, Groundable &g) {
  g.isGrounded = false;
  const auto tilemapBounds = map->GetBounds();
  // clamp x position to tilemap bounds
  t.position.x =
      glm::clamp(static_cast<int>(t.position.x),
                 static_cast<int>(tilemapBounds.x - c.vertices.x),
                 static_cast<int>(map->GetBounds().w - c.vertices.z));

  // collide with tiles
  SDL_Rect rect = {static_cast<int>(t.position.x + c.vertices.x),
                   static_cast<int>(t.position.y + c.vertices.y),
                   static_cast<int>(c.vertices.z - c.vertices.x),
                   static_cast<int>(c.vertices.w - c.vertices.y)};
  SDL_Rect found = {0, 0, 0, 0};
  bool isOverlapping = false;
  map->IsCollidingWith(&rect, found, e,
                       g.isGrounded); // check for collision

  if ((found.x != 0 || found.y != 0 || found.w != 0 || found.h != 0)) {
    push_rect_transform(rect, found, t, c);
  }
}

void LoadLevel(flecs::world &ecs, std::shared_ptr<Tilemap> map) {
  ecs.set<Map>({map});
}

void MapPlugin::addSystems(flecs::world &ecs) {
  // collision for entities with tilemap
  ecs.system<Transform2D, CollisionVolume, Groundable>().iter(
      [](flecs::iter it, Transform2D *t, CollisionVolume *c, Groundable *g) {
        Tilemap *map = it.world().get<Map>()->value.get();
        for (int i = 0; i < it.count(); i++) {
          flecs::entity e = it.entity(i);
          collideWithMap(map, e, t[i], c[i], g[i]);
        }
      });
}