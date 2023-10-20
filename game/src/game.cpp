#include "game.hpp"
#include <SDL.h>
#include <glad/glad.h>
#include <input.hpp>

#ifdef SHARED_GAME
#include <cassert>
#include <cr.h>

static Game game;

static int loaded_timestamp = 0;

CR_EXPORT int cr_main(struct cr_plugin *ctx, enum cr_op operation) {
  assert(ctx);

  gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress);
  // get a random int
  switch (operation) {
  case CR_LOAD:
    loaded_timestamp = SDL_GetTicks();
    game.init();
    return printf("loaded %i\n", loaded_timestamp);
  case CR_UNLOAD:
    game.unload();
    return printf("unloaded %i\n", loaded_timestamp);
  case CR_CLOSE:
    game.close();
    return printf("closed %i\n", loaded_timestamp);
  case CR_STEP:
    return game.update();
  }
  return 0;
}
#endif

Game::Game() {}

Game::~Game() {}

int Game::init() {
  SDL_SetWindowTitle(SDL_GL_GetCurrentWindow(), "Anya's World");
  // Get current window size
  int w, h;
  SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);
  this->spriteBatcher = std::make_unique<SpriteBatch>(glm::vec2(w, h));
  this->textureTink = std::make_shared<Texture>("assets/textures/tink.png");
  this->textureAnya = std::make_shared<Texture>("assets/textures/anya.png");

  return 0;
}

int Game::update() {
  int num_keys;
  const Uint8 *key_state = SDL_GetKeyboardState(&num_keys);
  InputManager::Update(key_state, num_keys);

  if (InputManager::GetKey(SDL_SCANCODE_SPACE).IsJustPressed()) {
    SDL_Log("Anya pressed");
  }

  glClearColor(0.25f, 0.25f, 0.25f, 1.0f);

  int w, h;
  SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);

  this->spriteBatcher->SetProjection(glm::vec2(w, h));
  this->spriteBatcher->SetDefaultView();

  this->spriteBatcher->DrawRect(glm::vec4(10, 0, 400, 400),
                                glm::vec4(1, 0, 0, 1.0f));

  this->spriteBatcher->Draw(this->textureTink.get(), glm::vec2(100, 100));
  this->spriteBatcher->Draw(this->textureAnya.get(), glm::vec2(0, 0));
  this->spriteBatcher->Flush();

  return 0;
}

int Game::unload() {
  this->spriteBatcher.reset();
  return 0;
}

int Game::close() {
  // clean up gl stuff
  return 0;
}
