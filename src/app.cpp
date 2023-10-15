#include "app.hpp"
#include <engine.hpp>

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

  game_logic();

#ifdef EMSCRIPTEN
  emscripten_set_main_loop(this->update, 0, this->is_running);
#else
  while (this->is_running) {
    this->update();
  }
#endif

#ifdef SHARED_GAME

  printf("Library path: %s\n", GAME_LIBRARY_PATH);

  cr_plugin game_ctx;
  cr_plugin_open(game_ctx, GAME_LIBRARY_PATH);

  while (this->is_running) {
    cr_plugin_update(game_ctx);
    fflush(stdout);
    fflush(stderr);
  }

  cr_plugin_close(game_ctx);
#else

  this->renderer.reset(); // destroy the renderer this has to be done before the
                          // window is destroyed

#endif
}

void App::update() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      this->is_running = false;
    }
  }

  this->renderer->Clear();

  this->renderer->Present();
}
