#include "app.hpp"
#include <engine.hpp>

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

  render("We can do some engine setup stuff here");

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
  game_logic();
#endif
}
