#pragma once

#include <memory>

#include "renderer.hpp"
#include "window.hpp"

class App {
public:
  App();
  ~App();
  void run();
  void update();

private:
  bool is_running;
  std::unique_ptr<Window> window;
  std::unique_ptr<Renderer> renderer;
};