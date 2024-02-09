#include "plugins/camera.hpp"
#include "plugins/map.hpp"
#include "plugins/player.hpp"

void CameraPlugin::addSystems(flecs::world &world) {
  const auto playerQuery =
      world.query<Transform2D, Player>(); // note this has to be declared
  // outside for emscripten

  world.system<Camera>().iter([playerQuery](flecs::iter it, Camera *c) {
    playerQuery.each([&c](Transform2D &t, Player &p) {
      const auto playerRect = p.defaultRect;
      glm::vec2 offset = glm::vec2(playerRect.z, playerRect.w) / 2.0f;
      c[0].position = t.position + offset;
    });

    Tilemap *m = it.world().get<Map>()->value.get();
    SpriteBatch *r = it.world().get<Renderer>()->renderer;

    r->UpdateCamera(c[0].position, m->GetBounds());

    // silly but the tilemap needs to be drawn after the camera is updated
    // and before everything else to avoid jitter
    m->Draw(r); // draw the tilemap
  });
}
