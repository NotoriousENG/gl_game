#include "game.hpp"
#include <SDL.h>
#include <glad/glad.h>
#include <input.hpp>

#ifdef SHARED_GAME
#include <cassert>
#include <cr.h>

static Game game;

CR_EXPORT int cr_main(struct cr_plugin *ctx, enum cr_op operation) {
  assert(ctx);

  gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress);

  const int num = 0;
  // get a random int
  switch (operation) {
  case CR_LOAD:
    game.init();
    return printf("loaded %i\n", num);
  case CR_UNLOAD:
    game.unload();
    return printf("unloaded %i\n", num);
  case CR_CLOSE:
    game.close();
    return printf("closed %i\n", num);
  case CR_STEP:
    return game.update();
  }
  return 0;
}
#endif

Game::Game() {}

Game::~Game() {}

int Game::init() {
  SDL_SetWindowTitle(SDL_GL_GetCurrentWindow(), "Spenc's World");

  return 0;
}

int Game::update() {
  int num_keys;
  const Uint8 *key_state = SDL_GetKeyboardState(&num_keys);
  InputManager::Update(key_state, num_keys);

  if (InputManager::GetKey(SDL_SCANCODE_SPACE).IsJustPressed()) {
    SDL_Log("Anya pressed");
  }

  glClearColor(1.0f, 0.5f, 0.0f, 1.0f);

  return 0;
}

int Game::unload() { return 0; }

int Game::close() {
  // clean up gl stuff
  return 0;
}
