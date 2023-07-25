#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "net_manager.cpp"
#include "renderer.hpp"
#include "window.hpp"

#include "memory"

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
      "join", "room-RisingStuck",
      [&net](std::string id) {
        printf("connected to player %s\n", id.c_str());
        net->sendTo(id, "hi!");
      },
      [](std::string id, std::string message) { 
          printf("Got message: %s\n", message.c_str());
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
