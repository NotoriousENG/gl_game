#pragma once

#include <memory>

#include "renderer.hpp"
#include "window.hpp"
#ifndef SHARED_GAME
#include <game.hpp>
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

#ifndef SHARED_GAME
  Game game;
#endif
};