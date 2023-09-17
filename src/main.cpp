#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <memory>

#include "components.hpp"
#include "defs.hpp"
#include "flecs.h"
#include "input.hpp"
#include "renderer.hpp"
#include "sprite-batch.hpp"
#include "texture.hpp"
#include "window.hpp"

static std::unique_ptr<Renderer> renderer;
static bool running = true;
static std::unique_ptr<SpriteBatch> spriteBatcher;

static flecs::world world;

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
  Window window(GAME_NAME, WINDOW_WIDTH, WINDOW_HEIGHT);
  renderer = std::make_unique<Renderer>(&window);

  spriteBatcher =
      std::make_unique<SpriteBatch>(glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT));

  std::shared_ptr texture_anya =
      std::make_shared<Texture>("assets/textures/anya.png");

  std::shared_ptr texture_tink =
      std::make_shared<Texture>("assets/textures/tink.png");

  std::string name = "Anya";
  int count = 0;

  auto Anya =
      world.prefab("Anya")
          .set<Transform2D>(Transform2D(glm::vec2(0, 0), glm::vec2(1, 1), 0))
          .set<Sprite>({texture_anya});

  auto Tink = world.prefab("Tink")
                  .set<Transform2D>(
                      Transform2D(glm::vec2(300, 300), glm::vec2(1, 1), 0))
                  .set<Sprite>({texture_tink})
                  .set<Player>({"Player 1"});

  // create entities
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 6; j++) {
      auto e =
          world.entity((name + " " + std::to_string(count)).c_str()).is_a(Anya);
      e.set<Transform2D>(
          Transform2D(glm::vec2(i * 100, j * 100), glm::vec2(1, 1), 0));
      count++;
    }
  }

  // create tink
  auto player = world.entity("player").is_a(Tink);

  // move anyas
  world.system<Transform2D>().without<Player>().iter(
      [](flecs::iter it, Transform2D *t) {
        for (int i : it) {
          t[i].rotation += 1.0f * it.delta_time();
        }
      });

  // player movement
  world.system<Player, Transform2D>().iter(
      [](flecs::iter it, Player *p, Transform2D *t) {
        const float speed = 200.0f;
        const glm::vec2 input = InputManager::GetVectorMovement();
        for (int i : it) {
          t[i].position += input * speed * it.delta_time();
        }
      });

  // render sprites
  world.system<Transform2D, Sprite>().iter(
      [](flecs::iter it, Transform2D *t, Sprite *s) {
        for (int i : it) {
          spriteBatcher->Draw(s[i].texture.get(), t[i].position, t[i].scale,
                              t[i].rotation);
        }
      });

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
  // sprite batcher must be destroyed before the renderer
  spriteBatcher.reset();
  // destroy the renderer this has to be done before the window is destroyed
  renderer.reset();

  return 0;
}
