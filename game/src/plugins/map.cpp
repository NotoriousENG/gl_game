#include "plugins/map.hpp"

#include <memory>

#include "resource-paths.hpp"
#include <asset-manager.hpp>

#include <asset-manager-aggregates.hpp>

#include <plugins/camera.hpp>
#include <plugins/enemy.hpp>
#include <plugins/graphics.hpp>
#include <plugins/map.hpp>
#include <plugins/physics.hpp>
#include <plugins/player.hpp>
#include <plugins/transform.hpp>
#include <prefabs.hpp>

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
  LockAllAssets();

  auto sb = ecs.get<Renderer>()->renderer;
  ecs.reset();
  ecs.reset();
  ecs.set_time_scale(0.0f);
  ecs.set<Camera>({.position = glm::vec2(0, 0)});
  ecs.set<Gravity>({.value = 980.0f});
  ecs.set<Renderer>({.renderer = sb});
  ecs.set<Map>({map});

  // Plugins
  PlayerPlugin().addSystems(ecs);
  EnemyPlugin().addSystems(ecs);
  PhysicsPlugin().addSystems(ecs);
  MapPlugin().addSystems(ecs);
  CameraPlugin().addSystems(ecs);
  Transform2DPlugin().addSystems(ecs);
  GraphicsPlugin().addSystems(ecs);

  const auto objects = map->GetObjects();

  const auto mapHeight = map->GetBounds().h;
  for (const auto &object : objects) {
    const auto pos = glm::vec2(object.getPosition().x, object.getPosition().y);
    if (object.getClass() == "PLAYER") {
      SDL_Log("Spawning player at %f, %f", pos.x, pos.y);
      SpawnPlayer(ecs, pos);
    } else if (object.getClass() == "ANYA") {
      SDL_Log("Spawning Anya at %f, %f", pos.x, pos.y);
      // get properties that are objects
      const auto properties = object.getProperties();
      Path path = Path({pos}, 1);
      for (const auto &property : properties) {
        if (property.getName() == "path") {
          const auto pathHandle = property.getObjectValue();
          const auto pathObject = map->GetObjectByHandle(pathHandle);
          const auto pathPoints = pathObject.getPoints();
          const auto pathPos =
              glm::vec2(pathObject.getPosition().x, pathObject.getPosition().y);
          std::vector<glm::vec2> points;
          for (const auto &pp : pathPoints) {
            const glm::vec2 point = {pp.x + pathPos.x, pp.y + pathPos.y};
            points.push_back(point);
          }
          path = Path(points, 1);
        }
      }
      SpawnAnya(ecs, pos, path);
    }
  }

  UnlockAllAssets();

  // set flecs time to 1 since we are done loading
  ecs.set_time_scale(1.0f);
}

void DrawColliders(flecs::world &ecs, SpriteBatch *sb) {
  ecs.query<Transform2D, CollisionVolume>().iter(
      [sb](flecs::iter it, Transform2D *tl, CollisionVolume *cl) {
        for (int i : it) {
          const auto e = it.entity(i);
          const auto c = cl[i];
          const auto t = tl[i];
          const auto world = it.world();
          // the rect is the vertices with the position offset
          const auto rect =
              c.vertices + glm::vec4(t.global_position.x, t.global_position.y,
                                     -c.vertices.x, -c.vertices.y);
          const bool isGrounded =
              e.has<Groundable>() && e.get<Groundable>()->isGrounded;

          auto color = isGrounded
                           ? glm::vec4(1, 0, 0, 0.5f)
                           : (it.world().get<Map>()->value->HasCollision(e)
                                  ? glm::vec4(0, 1, 0, 0.5f)
                                  : glm::vec4(0, 0, 1, 0.5f));
          if (e.has<Hurtbox>()) {
            if (e.get<Hurtbox>()->active) {
              color = glm::vec4(1, 1, 0, 0.5f);
            } else {
              color = glm::vec4(0, 0, 0, 0.5f);
            }
          }
          sb->DrawRect(rect, color);
        }
      });

  ecs.get<Map>()->value->DrawColliders(sb);
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