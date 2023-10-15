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
#include "debug.hpp"

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

  if (InputManager::GetKey(SDL_SCANCODE_1).IsJustPressed()) {
      DebugManager::ToggleRenderColliders();
  }

  // Clear the screen
  renderer->Clear();

  // run systems
  world.progress();

  if (DebugManager::GetRenderColliders()) {
    // Draw colliders
    world.query<Transform2D, Collider>().each([](flecs::entity e,
                                                  Transform2D &t, Collider &c) {
      // the rect is the vertices with the position offset
      const auto rect = c.vertices + glm::vec4(t.position.x, t.position.y,
                                               -c.vertices.x, -c.vertices.y);
      const auto color =
          c.isGrounded ? glm::vec4(1, 0, 0, 0.5f)
                       : (tilemap->HasCollision(e) ? glm::vec4(0, 1, 0, 0.5f)
                                                   : glm::vec4(0, 0, 1, 0.5f));
      spriteBatcher->DrawRect(rect, color);
    });
  }


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
  const auto Tink = world.prefab("Tink")
                        .set<Transform2D>(Transform2D(glm::vec2(300, 400),
                                                      glm::vec2(1, 1), 0))
                        .set<Sprite>({texture_tink})
                        .set<Player>({"Player 1"})
                        .set<Velocity>({glm::vec2(0, 0)})
                        .set<Collider>({glm::vec4(17, 7, 46, 57),
                                        ColliderType::SOLID, false});

  const auto Anya = world.prefab("Anya")
                        .set<Transform2D>(Transform2D(glm::vec2(150, 454),
                                                      glm::vec2(1, 1), 0))
                        .set<Sprite>({texture_anya})
                        .set<Velocity>({glm::vec2(0, 0)})
                        .set<Collider>({glm::vec4(17, 7, 46, 57),
                                        ColliderType::SOLID, false});

  // create anya
  const auto anya = world.entity("anya").is_a(Anya);

  // create tink
  const auto player = world.entity("player").is_a(Tink);

  world.set<Camera>({.position = glm::vec2(0, 0)});
  world.set<Gravity>({.value = 980.0f});

  // player movement + update camera
  world.system<Player, Velocity, Sprite, Collider>().iter(
      [](flecs::iter it, Player *p, Velocity *v, Sprite *s, Collider *c) {
        const float speed = 200.0f;
        const float move_x = InputManager::GetAxisHorizontalMovement();
        const auto jump = InputManager::GetTriggerJump();
        for (int i : it) {
          v[i].value.x = move_x * speed;
          if (c[i].isGrounded && jump) {
            v[i].value.y = -515.0f;
            c[i].isGrounded = false;
          }
        }
      });

  // gravity system
  world.system<Velocity, Collider>().iter(
      [](flecs::iter it, Velocity *v, Collider *c) {
        const auto dt = it.delta_time();
        const float g = world.get<Gravity>()->value;
        for (int i : it) {
          if (c[i].isGrounded) {
            v[i].value.y = 0.0f;
          } else {
            v[i].value.y += g * dt;
          }
        }
      });

  // velocity system
  world.system<Velocity, Transform2D>().iter(
      [](flecs::iter it, Velocity *v, Transform2D *t) {
        const auto dt = it.delta_time();
        for (int i : it) {
          t[i].position += v[i].value * dt;
        }
      });

  // collision for entities with tilemap
  world.system<Transform2D, Collider>().each(
      [](flecs::entity e, Transform2D &t, Collider &c) {
        c.isGrounded = false; // reset grounded state
        const auto tilemapBounds = tilemap->GetBounds();
        // clamp x position to tilemap bounds
        t.position.x =
            glm::clamp(static_cast<int>(t.position.x),
                       static_cast<int>(tilemapBounds.x - c.vertices.x),
                       static_cast<int>(tilemap->GetBounds().w - c.vertices.z));

        // collide with tiles
        SDL_Rect rect = {static_cast<int>(t.position.x + c.vertices.x),
                         static_cast<int>(t.position.y + c.vertices.y),
                         static_cast<int>(c.vertices.z - c.vertices.x),
                         static_cast<int>(c.vertices.w - c.vertices.y)};
        SDL_Rect found = {0, 0, 0, 0};
        bool isOverlapping = false;
        tilemap->IsCollidingWith(&rect, found, e,
                                 c.isGrounded); // check for collision

        if ((found.x != 0 || found.y != 0 || found.w != 0 || found.h != 0)) {
          push_rect_transform(rect, found, t, c);
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

      if (SDL_HasIntersection(&rect1, &rect2)) {
        push_rect_transform(rect1, rect2, t1, c1);
      }

      // if there is an intersection with a rect slightly above the second
      // rect then the player is grounded
      const SDL_Rect rect3 = {rect2.x + 3, rect2.y - 1, rect2.w - 7, 1};
      if (SDL_HasIntersection(&rect1, &rect3)) {
        c1.isGrounded = true;
      }
    });
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
  });

  // render sprites
  world.system<Transform2D, Sprite>().each([](Transform2D &t, Sprite &s) {
    spriteBatcher->Draw(s.texture.get(), t.position, t.scale, t.rotation);
  });

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
