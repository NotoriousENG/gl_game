#include "game.hpp"
#include "resource-paths.hpp"
#include <SDL.h>
#include <asset-manager.hpp>
#include <components.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <input.hpp>
#include <plugins/camera.hpp>
#include <plugins/enemy.hpp>
#include <plugins/graphics.hpp>
#include <plugins/map.hpp>
#include <plugins/physics.hpp>
#include <plugins/player.hpp>
#include <plugins/transform.hpp>
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

Game::Game() {
  game = this;
  world.set_time_scale(0.0f); // stop time while loading
}

Game::~Game() {}

int Game::init(SharedData *shared_data) {
  SDL_Log("Game init");
  SDL_SetWindowTitle(SDL_GL_GetCurrentWindow(), "Tink's World");
  world.set_time_scale(0.0f); // stop time while loading

  // map the text_input_buffer
  InputManager::SetTextInputBuffer(&shared_data->text_input_buffer[0]);
  // Get current window size
  int w, h;
  SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);
  this->spriteBatcher = std::make_unique<SpriteBatch>(glm::vec2(w, h));
  this->mixer = std::make_unique<Mixer>();
  auto textureAnya = AssetManager<Texture>::get(RES_TEXTURE_AMIIBO);
  auto textureArrow = AssetManager<Texture>::get(RES_TEXTURE_ARROW);
  auto textureBall = AssetManager<Texture>::get(RES_TEXTURE_BALL);
  auto tilemap = AssetManager<Tilemap>::get(RES_TILEMAP_DEMO);
  auto tilemap2 = AssetManager<Tilemap>::get(RES_TILEMAP_DEMO2);
  auto spritesheet = AssetManager<SpriteSheet>::get(RES_SHEET_PLAYER);
  auto music = AssetManager<Music>::get(RES_MUSIC_PLEASANT_CREEK);
  auto soundEffect = AssetManager<SoundEffect>::get(RES_SFX_MEOW);

  this->fontL = std::make_unique<Font>(RES_FONT_VERA, 32);
  this->fontS = std::make_unique<Font>(RES_FONT_VERA, 14);

  music->play_on_loop();

#ifndef EMSCRIPTEN
  this->mixer->ToggleMute();
#endif
  // prefabs
  const auto Tink =
      world.prefab("Tink")
          .set<Transform2D>(Transform2D(glm::vec2(80, 400), glm::vec2(1, 1), 0))
          .set<AnimatedSprite>(
              AnimatedSprite(spritesheet, spritesheet->GetAnimation("Idle")))
          .set<Player>({"Player 1", false, soundEffect, music,
                        spritesheet->GetAtlasRect(0)})
          .set<Velocity>({glm::vec2(0, 0)})
          .set<CollisionVolume>({glm::vec4(3, 7, 39, 39)})
          .set<Groundable>({false})
          .add<PhysicsBody>();

  const auto Anya = world.prefab("Anya")
                        .set<Transform2D>(Transform2D(glm::vec2(300, 510),
                                                      glm::vec2(1, 1), 0))
                        .set<Sprite>({textureAnya})
                        .set<Velocity>({glm::vec2(0, 0)})
                        .set<CollisionVolume>({
                            glm::vec4(0, 32, 64, 64),
                        })
                        .set<Health>({1.0f})
                        .set<Enemy>(Enemy())
                        .set<Path>(Path(
                            {
                                glm::vec2(300, 511),
                                glm::vec2(700, 511),
                            },
                            1));

  const auto Ball =
      world.prefab("Ball")
          .set<Transform2D>(Transform2D(glm::vec2(0, 0), glm::vec2(1, 1), 0))
          .set<Sprite>({textureBall})
          .set<Velocity>({glm::vec2(0, 0)})
          .set<Groundable>({false})
          .set<CollisionVolume>({
              glm::vec4(0, 0, 16, 16),
          })
          .set<LiveFor>({5.0f});

  const auto HpBar = world.prefab("UIFilledRect")
                         .set<Transform2D>(Transform2D(glm::vec2(0.0f, -15.0f),
                                                       glm::vec2(1, 1), 0))
                         .set<UIFilledRect>(UIFilledRect(
                             glm::vec2(50.0f, 5.0f), 1.0f, 1.0f,
                             glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, 0, 0.3f)));

  const auto DirectionArrow =
      world.prefab("DirectionIndicator")
          .set<Transform2D>(
              Transform2D(glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), 0))
          .set<Sprite>({textureArrow});

  const auto TextArea =
      world.prefab("textArea")
          .set<Transform2D>(
              Transform2D(glm::vec2(-45.0f, -200.0), glm::vec2(1, 1), 0))
          .set<UIFilledRect>(UIFilledRect(glm::vec2(128.0f, 128.0f), 1.0f, 1.0f,
                                          glm::vec4(1, 1, 1, 1),
                                          glm::vec4(0, 0, 0, 0.3f)))
          .set<AdjustingTextBox>(AdjustingTextBox(
              this->fontS.get(), InputManager::GetTextInputBuffer(), 0));
  // create anya
  // add 1 anya per 64 units on x
  std::string anya_name = "anya";
  world.entity(anya_name.c_str()).is_a(Anya);

  // create tink
  const auto player = world.entity("player").is_a(Tink);
  const auto hpBar = world.entity("hpBar").is_a(HpBar).child_of(player);
  const auto directionArrow =
      world.entity("directionArrow").is_a(DirectionArrow).child_of(player);
  const auto textArea =
      world.entity("playerChatbox").is_a(TextArea).child_of(player);

  // add hurtbox as a child of tink
  const auto hurtbox =
      world.entity("hurtbox")
          .child_of(player)
          .set<Transform2D>(Transform2D(glm::vec2(30, 0), glm::vec2(1, 1), 0))
          .set<CollisionVolume>({glm::vec4(3, 7, 30, 48)})
          .set<Hurtbox>({1.0f, false});

  world.set<Camera>({.position = glm::vec2(0, 0)});
  world.set<Gravity>({.value = 980.0f});
  world.set<Renderer>({.renderer = this->spriteBatcher.get()});
  LoadLevel(this->world, tilemap);

  // Plugins
  PlayerPlugin().addSystems(this->world);
  EnemyPlugin().addSystems(this->world);
  PhysicsPlugin().addSystems(this->world);
  MapPlugin().addSystems(this->world);
  CameraPlugin().addSystems(this->world);
  Transform2DPlugin().addSystems(this->world);
  GraphicsPlugin().addSystems(this->world);

  // set flecs time to 1 since we are done loading
  world.set_time_scale(1.0f);

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
  }
#endif

  this->world.progress();

  if (this->drawColliders) {
    // Draw colliders
    world.query<Transform2D, CollisionVolume>().iter(
        [](flecs::iter it, Transform2D *tl, CollisionVolume *cl) {
          for (int i : it) {
            const auto e = it.entity(i);
            const auto c = cl[i];
            const auto t = tl[i];
            const auto world = it.world();
            // the rect is the vertices with the position offset
            const auto rect =
                c.vertices + glm::vec4(t.global_position.x, t.global_position.y,
                                       -c.vertices.x, -c.vertices.y);
            const bool isGrounded =
                e.has<Groundable>() && e.get<Groundable>()->isGrounded;

            auto color = isGrounded ? glm::vec4(1, 0, 0, 0.5f)
                                    : (world.get<Map>()->value->HasCollision(e)
                                           ? glm::vec4(0, 1, 0, 0.5f)
                                           : glm::vec4(0, 0, 1, 0.5f));
            if (e.has<Hurtbox>()) {
              if (e.get<Hurtbox>()->active) {
                color = glm::vec4(1, 1, 0, 0.5f);
              } else {
                color = glm::vec4(0, 0, 0, 0.5f);
              }
            }
            game->spriteBatcher->DrawRect(rect, color);
          }
        });
    world.get<Map>()->value->DrawColliders(game->spriteBatcher.get());
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
