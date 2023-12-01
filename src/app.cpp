#define CR_HOST

#include "app.hpp"

#include <defs.hpp>
#include <glm/glm.hpp>

#include <stdio.h>

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
  cr_plugin_open(this->game_ctx, GAME_LIBRARY_PATH);
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
  if (cr_plugin_changed(
          this->game_ctx)) { // full teardown needed on non windows
    cr_plugin_close(this->game_ctx);
    cr_plugin_open(this->game_ctx, GAME_LIBRARY_PATH);
  }
  cr_plugin_update(this->game_ctx);
  fflush(stdout);
  fflush(stderr);
#else
  this->game.update();
#endif
  this->renderer->Present();
}

void App::onClose() {
#ifdef SHARED_GAME
  cr_plugin_close(this->game_ctx);
  SDL_Log("App closed\n");
#else
  this->game.unload();
  this->game.close();
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
