#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <memory>

#include "defs.hpp"
#include "renderer.hpp"
#include "sprite-batch.hpp"
#include "texture.hpp"
#include "window.hpp"

static std::unique_ptr<Renderer> renderer;
static bool running = true;
static std::shared_ptr<Texture> texture_anya;
static std::unique_ptr<SpriteBatch> spriteBatcher;

void main_loop() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      running = false;
    }
  }

  // Clear the screen
  renderer->Clear();

  // Render sprites
  // ...
  // Loop and draw 1000 sprites to the screen
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      for (int k = 0; k < 10; k++) {
        spriteBatcher->Draw(

            // position of the rectangle
            glm::vec4(100 + i * 50 + k * -10, 100 + j * 50 + k * -10, 50, 50),

            // rectangle size
            glm::vec4(0, 0, 300, 320),

            // color to tint the sprite
            glm::vec4(i / 10.f, j / 10.f, k / 10.f, 1),

            // texture of the sprite
            texture_anya.get());

        // Uncomment this line to see how much slower it is to call draw
        // separately for each sprite.
        // spriteBatcher->Flush();
      }
    }
  }
  // Now that we have a collection of all the draws we want to make, send it all
  // to the gpu to be drawn!
  spriteBatcher->Flush();

  // Swap buffers
  renderer->Present();
}

int main(int argc, char *argv[]) {
  Window window(GAME_NAME, WINDOW_WIDTH, WINDOW_HEIGHT);
  renderer = std::make_unique<Renderer>(&window);

  spriteBatcher =
      std::make_unique<SpriteBatch>(glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT));

  texture_anya = std::make_shared<Texture>("assets/textures/anya.png");

  // std::shared_ptr<Texture> texture_tink =
  //     std::make_shared<Texture>("assets/textures/tink.png");

  // Main loop
  running = true;
#ifdef EMSCRIPTEN
  emscripten_set_main_loop(main_loop, 0, running);
#else
  while (running) {
    main_loop();
  }
#endif

  // need opengl context to free texture
  texture_anya.reset();
  // sprite batcher must be destroyed before the renderer
  spriteBatcher.reset();
  // destroy the renderer this has to be done before the window is destroyed
  renderer.reset();

  return 0;
}
