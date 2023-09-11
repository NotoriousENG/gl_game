#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <memory>

#include "components.hpp"
#include "defs.hpp"
#include "flecs.h"
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

  std::string name = "Anya";
  int count = 0;

  // create entities
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      auto e = world.entity((name + " " + std::to_string(count)).c_str());
      e.set<Transform2D>(
          Transform2D(glm::vec2(i * 100, j * 100), glm::vec2(1, 1), 0));
      e.set<Sprite>({texture_anya});
      count++;
    }
  }

  // mount systems
  world.system<Transform2D, Sprite>().iter(
      [](flecs::iter it, Transform2D *t, Sprite *s) {
        for (int i : it) {
          spriteBatcher->Draw(s[i].texture.get(), t[i].position, t[i].scale,
                              t[i].rotation);
        }
      });

  world.system<Transform2D>().iter([](flecs::iter it, Transform2D *t) {
    for (int i : it) {
      t[i].rotation += 1.0f * it.delta_time();
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
