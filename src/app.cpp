#include "app.hpp"

#include <defs.hpp>
#include <glm/glm.hpp>

#include <stdio.h>

#ifdef SHARED_GAME
#define CR_HOST
#include <cr.h>
#else
#include <game.hpp>
#endif

App::App() { this->is_running = true; }

App::~App() {}

void App::run() {

  const auto initial_window_size = glm::vec2(800, 600);
  this->window = std::make_unique<Window>(GAME_NAME, initial_window_size.x,
                                          initial_window_size.y);
  this->renderer = std::make_unique<Renderer>(this->window.get());

#ifdef EMSCRIPTEN
  emscripten_set_main_loop(this->update, 0, this->is_running);
#endif

#ifdef SHARED_GAME

  printf("Library path: %s\n", GAME_LIBRARY_PATH);

  cr_plugin game_ctx;
  cr_plugin_open(game_ctx, GAME_LIBRARY_PATH);

  while (this->is_running) {
    this->renderer->Clear();
    this->update();
    cr_plugin_update(game_ctx);
    fflush(stdout);
    fflush(stderr);
    this->renderer->Present();
  }

  cr_plugin_close(game_ctx);
#else
  while (this->is_running) {
    this->renderer->Clear();
    this->update();
    game_init();
    this->renderer->Present();
  }
#endif
  this->renderer.reset(); // destroy the renderer this has to be done before the
                          // window is destroyed
}

void App::update() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      this->is_running = false;
    }
  }
}
