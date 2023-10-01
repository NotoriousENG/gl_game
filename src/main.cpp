#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "components.hpp"
#include "defs.hpp"
#include "flecs.h"
#include "input.hpp"
#include "net_manager.hpp"
#include "renderer.hpp"
#include "sprite-batch.hpp"
#include "texture.hpp"
#include "tilemap.hpp"
#include "window.hpp"
#include <SDL2/SDL_log.h>
#include <memory>

static std::unique_ptr<Renderer> renderer;
static bool running = true;
static std::unique_ptr<SpriteBatch> spriteBatcher;

static flecs::world world;

static std::unique_ptr<Tilemap> tilemap;

void push_rect_transform(const SDL_Rect &rect, const SDL_Rect &pushedBy,
                         Transform2D &t1, Collider &c1) {
  const auto rect1Center = glm::vec2(rect.x + rect.w / 2, rect.y + rect.h / 2);

  const auto rect2Center =
      glm::vec2(pushedBy.x + pushedBy.w / 2, pushedBy.y + pushedBy.h / 2);

  const auto distanceX = rect1Center.x - rect2Center.x;
  const auto distanceY = rect1Center.y - rect2Center.y;

  // Figure out the combined half-widths and half-heights
  const auto halfWidths = (rect.w + pushedBy.w) / 2;
  const auto halfHeights = (rect.h + pushedBy.h) / 2;

  // Calculate the overlap between the two rectangles
  const auto overlapX = halfWidths - std::abs(distanceX);
  const auto overlapY = halfHeights - std::abs(distanceY);

  // The collision has happened on the axis of least penetration
  const bool isMoreVertical = overlapX > overlapY;

  if (isMoreVertical) {
    if (distanceY < 0) {
      t1.position.y = pushedBy.y - rect.h - c1.vertices.y;
    } else {
      t1.position.y = pushedBy.y + pushedBy.h - c1.vertices.y;
    }
  } else {
    if (distanceX < 0) {
      t1.position.x = pushedBy.x - rect.w - c1.vertices.x;
    } else {
      t1.position.x = pushedBy.x + pushedBy.w - c1.vertices.x;
    }
  }
}

void main_loop() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      running = false;
    }
  }
  int num_keys;
  const Uint8 *key_state = SDL_GetKeyboardState(&num_keys);
  InputManager::Update(key_state, num_keys);

  // Clear the screen
  renderer->Clear();

  // run systems
  world.progress();

  // draw all sprites in the batch
  spriteBatcher->Flush();

  // Swap buffers
  renderer->Present();
}

int main(int argc, char *argv[]) {
  const auto initial_window_size = glm::vec2(800, 600);
  Window window(GAME_NAME, initial_window_size.x, initial_window_size.y);
  renderer = std::make_unique<Renderer>(&window);

  spriteBatcher = std::make_unique<SpriteBatch>(
      glm::vec2(initial_window_size.x, initial_window_size.y));

  std::shared_ptr texture_anya =
      std::make_shared<Texture>("assets/textures/anya.png");

  std::shared_ptr texture_tink =
      std::make_shared<Texture>("assets/textures/tink.png");

  tilemap = std::make_unique<Tilemap>("assets/tilemaps/demo.tmx");

  // prefabs
  const auto Tink =
      world.prefab("Tink")
          .set<Transform2D>(
              Transform2D(glm::vec2(300, 400), glm::vec2(1, 1), 0))
          .set<Sprite>({texture_tink})
          .set<Player>({"Player 1"})
          .set<Collider>({glm::vec4(17, 7, 46, 57), ColliderType::SOLID});

  const auto Anya =
      world.prefab("Anya")
          .set<Transform2D>(
              Transform2D(glm::vec2(150, 454), glm::vec2(1, 1), 0))
          .set<Sprite>({texture_anya})
          .set<Collider>({glm::vec4(17, 7, 46, 57), ColliderType::SOLID});

  // create anya
  const auto anya = world.entity("anya").is_a(Anya);

  // create tink
  const auto player = world.entity("player").is_a(Tink);

  world.set<Camera>({.position = glm::vec2(0, 0)});

  // player movement + update camera
  world.system<Player, Transform2D, Sprite>().iter(
      [](flecs::iter it, Player *p, Transform2D *t, Sprite *s) {
        const float speed = 200.0f;
        const glm::vec2 input = InputManager::GetVectorMovement();
        for (int i : it) {
          t[i].position += input * speed * it.delta_time();
        }
      });

  const auto collisionQuery = world.query<Transform2D, Collider>();
  // Colision Between player + entities
  world.system<Transform2D, Collider, Player>().each([&collisionQuery](
                                                         flecs::entity e1,
                                                         Transform2D &t1,
                                                         Collider &c1,
                                                         Player &p1) {
    collisionQuery.each([&e1, &t1, &c1](flecs::entity e2, Transform2D &t2,
                                        Collider &c2) {
      if (e1.id() == e2.id()) {
        return;
      }
      const SDL_Rect rect1 = {static_cast<int>(t1.position.x + c1.vertices.x),
                              static_cast<int>(t1.position.y + c1.vertices.y),
                              static_cast<int>(c1.vertices.z - c1.vertices.x),
                              static_cast<int>(c1.vertices.w - c1.vertices.y)};
      const SDL_Rect rect2 = {static_cast<int>(t2.position.x + c2.vertices.x),
                              static_cast<int>(t2.position.y + c2.vertices.y),
                              static_cast<int>(c2.vertices.z - c2.vertices.x),
                              static_cast<int>(c2.vertices.w - c2.vertices.y)};
      //// Log both rects
      // SDL_Log("rect1: %i, %i, %i, %i\n", rect1.x, rect1.y, rect1.w,
      // rect1.h); SDL_Log("rect2: %i, %i, %i, %i\n", rect2.x, rect2.y,
      // rect2.w, rect2.h);

      if (SDL_HasIntersection(&rect1, &rect2)) {
        // Calculate the horizontal and vertical distances between the
        // centers of the collider rectangles
        push_rect_transform(rect1, rect2, t1, c1);
      }
    });
  });

  // collision for entities with tilemap
  world.system<Transform2D, Collider>().each([](Transform2D &t, Collider &c) {
    const auto tilemapBounds = tilemap->GetBounds();
    // clamp x position to tilemap bounds
    t.position.x =
        glm::clamp(static_cast<int>(t.position.x),
                   static_cast<int>(tilemapBounds.x - c.vertices.x),
                   static_cast<int>(tilemap->GetBounds().w - c.vertices.z));
    // since we want to be able to fall off the map don't worry about y
    // t.position.y =
    //     glm::clamp(static_cast<int>(t.position.y),
    //                static_cast<int>(tilemapBounds.y - c.vertices.y),
    //                static_cast<int>(tilemap->GetBounds().h - c.vertices.w));

    // collide with tiles
    SDL_Rect rect = {static_cast<int>(t.position.x + c.vertices.x),
                     static_cast<int>(t.position.y + c.vertices.y),
                     static_cast<int>(c.vertices.z - c.vertices.x),
                     static_cast<int>(c.vertices.w - c.vertices.y)};
    SDL_Rect found = {0, 0, 0, 0};
    tilemap->IsCollidingWith(&rect, found);
    if (found.x != 0 || found.y != 0 || found.w != 0 || found.h != 0) {
      // draw the collider as a red rect
      push_rect_transform(rect, found, t, c);
    }
  });

  // any rendering should happen after logic

  const auto playerQuery = world.query<Sprite, Transform2D, Player>();

  // update sprite batcher uniforms from camera
  world.system<Camera>().each([playerQuery](Camera &c) {
    playerQuery.each([&c](Sprite &s, Transform2D &t, Player &p) {
      const auto rect = s.texture->GetTextureRect();
      glm::vec2 offset = glm::vec2(rect.z, rect.w) / 2.0f;
      c.position = t.position + offset;
    });
    spriteBatcher->UpdateCamera(c.position, tilemap->GetBounds());

    // silly but the tilemap needs to be drawn after the camera is updated
    // and before everything else to avoid jitter
    tilemap->Draw(spriteBatcher.get()); // draw the tilemap
    // draw all sprites in the batch
    spriteBatcher->Flush();
  });

  // render sprites
  world.system<Transform2D, Sprite>().each([](Transform2D &t, Sprite &s) {
    spriteBatcher->Draw(s.texture.get(), t.position, t.scale, t.rotation);
  });

  if (DEBUG_COLLISIONS) {
    // Draw colliders
    world.system<Transform2D, Collider>().each([](Transform2D &t, Collider &c) {
      // the rect is the vertices with the position offset
      const auto rect = c.vertices + glm::vec4(t.position.x, t.position.y,
                                               -c.vertices.x, -c.vertices.y);
      const auto color = c.type == ColliderType::SOLID
                             ? glm::vec4(0, 0, 1, 0.5f)
                             : glm::vec4(0, 1, 0, 0.5f);
      spriteBatcher->DrawRect(rect, color);
    });
  }

  // Get connections
  std::unique_ptr<NetManager> net = std::make_unique<NetManager>(
      "join", "room-RisingStuck",
      [&net](std::string id) {
        SDL_Log("connected to player %s\n", id.c_str());
        net->sendTo(id, "hi!");
      },
      [](std::string id, std::string message) {
        SDL_Log("Got message: %s\n", message.c_str());
      });
  net->connectToSignaling();

  // Main loop
  running = true;
#ifdef EMSCRIPTEN
  emscripten_set_main_loop(main_loop, 0, running);
#else
  while (running) {
    main_loop();
  }
#endif

  // reset the world to destruct all entities
  world.reset();

  // need opengl context to free texture
  texture_anya.reset();
  texture_tink.reset();
  tilemap.reset();
  // sprite batcher must be destroyed before the renderer
  spriteBatcher.reset();
  // destroy the renderer this has to be done before the window is destroyed
  renderer.reset();

  return 0;
}
