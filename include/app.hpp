#pragma once

#include <memory>

#include "renderer.hpp"
#include "window.hpp"

#ifdef SHARED_GAME
#include <cr.h>
#else
#include <game.hpp>
#endif

#ifdef EMSCRIPTEN
void emscripten_update();
#endif

class App {
public:
  App();
  ~App();
  void run();
  void update();
  void onClose();
  void poll_events();

private:
  bool is_running;
  std::unique_ptr<Window> window;
  std::unique_ptr<Renderer> renderer;

#ifdef SHARED_GAME
  cr_plugin game_ctx;
#else
  Game game;
#endif
};