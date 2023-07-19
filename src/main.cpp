#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <memory>

#include "renderer.hpp"
#include "window.hpp"
#include "defs.hpp"

static std::unique_ptr<Renderer> renderer;
static bool running = true;
static std::vector<Sprite> sprites;

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
  for (auto &sprite : sprites) {
    renderer->RenderSprite(sprite);
  }

  // Swap buffers
  renderer->Present();
}

int main(int argc, char *argv[]) {
  Window window(GAME_NAME, WINDOW_WIDTH, WINDOW_HEIGHT);
  renderer = std::make_unique<Renderer>(&window);

  GLuint texture_anya = renderer->LoadTexture("assets/textures/anya.png");
  GLuint texture_tink = renderer->LoadTexture("assets/textures/tink.png");

  sprites.push_back(Sprite{.texture = texture_anya,
            .position = glm::vec2(2.0f, 0.0f),
            .rotation = 0.0f,
            .scale = glm::vec2(1.0f, 1.0f)});

  sprites.push_back(Sprite{.texture = texture_tink,
            .position = glm::vec2(-2.0f, 0.0f),
            .rotation = 0.0f,
            .scale = glm::vec2(1.0f, 1.0f)});

  // Main loop
  running = true;
#ifdef EMSCRIPTEN
  emscripten_set_main_loop(main_loop, 0, running);
#else
  while (running) {
    main_loop();
  }
#endif

  // destroy the renderer this has to be done before the window is destroyed
  renderer.reset();

  glDeleteTextures(1, &texture_anya);
  glDeleteTextures(1, &texture_tink);
  SDL_Log("Texture deleted");

  return 0;
}
