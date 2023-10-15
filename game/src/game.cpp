#include "game.hpp"
#include <engine.hpp>

#ifdef SHARED_GAME
#include <cassert>
#include <cr.h>

CR_EXPORT int cr_main(struct cr_plugin *ctx, enum cr_op operation) {
  const int num = 2;
  assert(ctx);
  switch (operation) {
  case CR_LOAD:
    game_logic();
    return printf("loaded %i\n", num);
  case CR_UNLOAD:
    return printf("unloaded %i\n", num);
  case CR_CLOSE:
    return printf("closed %i\n", num);
  }
  // CR_STEP
  return 0; // update fn
}
#endif

void game_logic() { render("Game tells engine to render"); }