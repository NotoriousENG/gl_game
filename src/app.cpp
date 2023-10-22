#include "app.hpp"

#include <defs.hpp>
#include <glm/glm.hpp>

#include <stdio.h>

#ifdef SHARED_GAME
#define CR_HOST
#include <chrono>
#include <cr.h>
#include <thread>
#else
#include <game.hpp>
#endif

#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>

static App *app_instance;

void emscripten_update() { app_instance->update(); }
#endif

App::App() { this->is_running = true; }

App::~App() {}

void App::run() {

  const auto initial_window_size = glm::vec2(800, 600);
  this->window = std::make_unique<Window>(GAME_NAME, initial_window_size.x,
                                          initial_window_size.y);
  this->renderer = std::make_unique<Renderer>(this->window.get());

#ifdef SHARED_GAME
  SDL_Log("Library path: %s\n", GAME_LIBRARY_PATH);
  cr_plugin game_ctx;
  cr_plugin_open(game_ctx, GAME_LIBRARY_PATH);
#else
  this->game.init();
#endif

#ifdef EMSCRIPTEN
  app_instance = this;
  emscripten_set_main_loop(emscripten_update, 0, this->is_running);
#else
  while (this->is_running) {
    this->update();
  }
#endif

  this->onClose();
}

void App::update() {
  this->renderer->Clear();
  this->poll_events();
#ifdef SHARED_GAME
  cr_plugin_update(game_ctx);
  fflush(stdout);
  fflush(stderr);
#else
  game.update();
#endif
  this->renderer->Present();
}

void App::onClose() {
#ifdef SHARED_GAME
  cr_plugin_close(game_ctx);
#else
  game.unload();
  game.close();
#endif
  // destroy the renderer this has to be done before the
  // window is destroyed
  this->renderer.reset();
}

void App::poll_events() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      this->is_running = false;
    }
  }
}
