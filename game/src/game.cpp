#include "game.hpp"
#include "resource-paths.hpp"
#include <SDL.h>
#include <asset-manager.hpp>
#include <components.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <input.hpp>
#include <plugins/graphics.hpp>
#include <plugins/map.hpp>

#include <utils.hpp>

static Game *game; // this is dirty but it works for now

#ifdef SHARED_GAME
#include <cassert>
#include <cr.h>

static int loaded_timestamp = 0;

CR_EXPORT int cr_main(struct cr_plugin *ctx, enum cr_op operation) {
  assert(ctx);

  gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress);
  // get a random int
  switch (operation) {
  case CR_LOAD:
    loaded_timestamp = SDL_GetTicks();
    game = new Game();
    game->init((SharedData *)ctx->userdata);
    return printf("loaded %i\n", loaded_timestamp);
  case CR_UNLOAD:
    game->unload();
    return printf("unloaded %i\n", loaded_timestamp);
  case CR_CLOSE:
    game->close();
    delete game;
    return printf("closed %i\n", loaded_timestamp);
  case CR_STEP:
    return game->update();
  }
  return 0;
}
#endif

Game::Game() { game = this; }

Game::~Game() {}

int Game::init(SharedData *shared_data) {
  SDL_Log("Game init");
  SDL_SetWindowTitle(SDL_GL_GetCurrentWindow(), "Tink's World");

  // map the text_input_buffer
  InputManager::SetTextInputBuffer(&shared_data->text_input_buffer[0]);
  // Get current window size
  int w, h;
  SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);
  this->spriteBatcher = std::make_unique<SpriteBatch>(glm::vec2(w, h));
  this->mixer = std::make_unique<Mixer>();

#ifndef EMSCRIPTEN
  this->mixer->ToggleMute();
#endif

  const auto tilemap = AssetManager<Tilemap>::get(RES_TILEMAP_DEMO);
  world.set<Renderer>({.renderer = this->spriteBatcher.get()});
  LoadLevel(this->world, tilemap);

  return 0;
}

int Game::update() {
  int num_keys;
  const Uint8 *key_state = SDL_GetKeyboardState(&num_keys);
  InputManager::Update(key_state, num_keys);

  if (InputManager::GetKey(SDL_SCANCODE_RETURN).IsJustPressed()) {
    InputManager::ToggleTextInput();
  };

  // mute audio
  if (!InputManager::IsTextInputActive()) {
    if (InputManager::GetKey(SDL_SCANCODE_1).IsJustPressed()) {
      this->drawColliders = !this->drawColliders;
    };
    if (InputManager::GetKey(SDL_SCANCODE_M).IsJustPressed()) {
      this->mixer->ToggleMute();
    }
  }

  if (InputManager::GetKey(SDL_SCANCODE_F1).IsJustPressed()) {
    this->level1 = !this->level1;
    LoadLevel(this->world, this->level1
                               ? AssetManager<Tilemap>::get(RES_TILEMAP_DEMO)
                               : AssetManager<Tilemap>::get(RES_TILEMAP_DEMO2));
    return 0;
  }

#ifdef SHARED_GAME
  if (InputManager::GetKey(SDL_SCANCODE_F5).IsJustPressed()) {
    std::string path = "../../";
    std::string demoPath = path + RES_TILEMAP_DEMO;
    std::string demo2Path = path + RES_TILEMAP_DEMO2;
    // hot reload assets
    LoadLevel(this->world, this->level1
                               ? AssetManager<Tilemap>::get(demoPath.c_str())
                               : AssetManager<Tilemap>::get(demo2Path.c_str()));
    return 0;
  }
#endif

  this->world.progress();

  if (this->drawColliders) {
    DrawColliders(this->world, this->spriteBatcher.get());
  }
  // draw all sprites in the batch
  this->spriteBatcher->Flush();

  return 0;
}

int Game::unload() { return 0; }

int Game::close() {
  // clean up gl stuff
  return 0;
}
