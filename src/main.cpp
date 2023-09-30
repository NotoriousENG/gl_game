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

  tilemap->Draw(spriteBatcher.get()); // draw the tilemap
  // draw all sprites in the batch
  spriteBatcher->Flush();

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

  int count = 0;

  // prefabs
  auto Tink =
      world.prefab("Tink")
          .set<Transform2D>(
              Transform2D(glm::vec2(300, 300), glm::vec2(1, 1), 0))
          .set<Sprite>({texture_tink})
          .set<Player>({"Player 1"})
          .set<Collider>({glm::vec4(17, 7, 46, 57), ColliderType::SOLID});

  // create tink
  auto player = world.entity("player").is_a(Tink);

  world.set<Camera>({.position = glm::vec2(0, 0)});

  // player movement + update camera
  world.system<Player, Transform2D, Sprite>().iter(
      [](flecs::iter it, Player *p, Transform2D *t, Sprite *s) {
        const float speed = 200.0f;
        const glm::vec2 input = InputManager::GetVectorMovement();
        for (int i : it) {
          t[i].position += input * speed * it.delta_time();
        }
        const auto rect = s[0].texture->GetTextureRect();
        glm::vec2 offset = glm::vec2(rect.z, rect.w) / 2.0f;
        world.get_mut<Camera>()->position = t[0].position + offset;
      });

  // update sprite batcher uniforms from camera
  world.system<Camera>().iter([](flecs::iter it, Camera *c) {
    spriteBatcher->UpdateCamera(c->position);
  });

  // render sprites
  world.system<Transform2D, Sprite>().iter(
      [](flecs::iter it, Transform2D *t, Sprite *s) {
        for (int i : it) {
          spriteBatcher->Draw(s[i].texture.get(), t[i].position, t[i].scale,
                              t[i].rotation);
        }
      });

  // Draw colliders
  world.system<Transform2D, Collider>().iter(
      [](flecs::iter it, Transform2D *t, Collider *c) {
        for (int i : it) {
          // the rect is the vertices with the position offset
          const auto rect =
              c[i].vertices + glm::vec4(t[i].position.x, t[i].position.y,
                                        -c[i].vertices.x, -c[i].vertices.y);
          const auto color = c[i].type == ColliderType::SOLID
                                 ? glm::vec4(0, 0, 1, 0.5f)
                                 : glm::vec4(0, 1, 0, 0.5f);
          spriteBatcher->DrawRect(nullptr, rect, color);
        }
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
