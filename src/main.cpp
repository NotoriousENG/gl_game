#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "net_manager.cpp"
#include "renderer.hpp"
#include "window.hpp"

#include "memory"
#include <iostream>

static std::unique_ptr<Renderer> renderer;
static bool running = true;
static Sprite sprite;

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
  renderer->RenderSprite(sprite);

  // Swap buffers
  renderer->Present();
}

int main(int argc, char *argv[]) {
  Window window(GAME_NAME, 800, 600);
  renderer = std::make_unique<Renderer>(&window);

  GLuint texture = renderer->LoadTexture("assets/textures/texture.png");

  sprite = {.texture = texture,
            .position = glm::vec2(0.0f, 0.0f),
            .size = glm::vec2(64.0f, 64.0f)};

  // Get connections
  NetManager *net;
  net = new NetManager(
      argv[1],
      argv[2],
      [&net](std::string id) {
        std::cout << "connected to player: " << id << std::endl;
        net->sendTo(id, "hi!");
      },
      [](std::string id, std::string message) {
        std::cout << "message from player: " << id << "is: " << message << std::endl;
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

  // destroy the renderer this has to be done before the window is destroyed
  renderer.reset();

  glDeleteTextures(1, &texture);
  SDL_Log("Texture deleted");

  return 0;
}
