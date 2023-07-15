#include "renderer.hpp"
#include "window.hpp"

int main(int argc, char *argv[]) {
  Window window(GAME_NAME, 800, 600);
  Renderer renderer(&window);

  GLuint texture = renderer.LoadTexture("assets/textures/texture.png");

  Sprite sprite = {.texture = texture,
                   .position = glm::vec2(0.0f, 0.0f),
                   .size = glm::vec2(64.0f, 64.0f)};

  // Main loop
  bool quit = false;
  while (!quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
      }
    }

    // Clear the screen
    renderer.Clear();

    // Render sprites
    // ...
    renderer.RenderSprite(sprite);

    // Swap buffers
    renderer.Present();
  }

  glDeleteTextures(1, &texture);
  SDL_Log("Texture deleted");

  return 0;
}
