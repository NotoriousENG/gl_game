#include "game.hpp"
#include <SDL.h>
#include <components.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <input.hpp>
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

void Game::push_rect_transform(const SDL_Rect &rect, const SDL_Rect &pushedBy,
                               Transform2D &t1, CollisionVolume &c1) {
  const auto rect1Center = glm::vec2(rect.x + rect.w / 2, rect.y + rect.h / 2);

  const auto rect2Center =
      glm::vec2(pushedBy.x + pushedBy.w / 2, pushedBy.y + pushedBy.h / 2);

  const auto distanceX = rect1Center.x - rect2Center.x;
  const auto distanceY = rect1Center.y - rect2Center.y;

  // Figure out the combined half-widths and half-heights
  const auto halfWidths = (rect.w + pushedBy.w) / 2;
  const auto halfHeights = (rect.h + pushedBy.h) / 2;

  // Calculate the overlap between the two rectangles
  const auto overlapX = halfWidths - std::abs(distanceX);
  const auto overlapY = halfHeights - std::abs(distanceY);

  // The collision has happened on the axis of least penetration
  const bool isMoreVertical = overlapX > overlapY;

  if (isMoreVertical) {
    if (distanceY < 0) {
      t1.position.y = pushedBy.y - rect.h - c1.vertices.y;
    } else {
      t1.position.y = pushedBy.y + pushedBy.h - c1.vertices.y;
    }
  } else {
    if (distanceX < 0) {
      t1.position.x = pushedBy.x - rect.w - c1.vertices.x;
    } else {
      t1.position.x = pushedBy.x + pushedBy.w - c1.vertices.x;
    }
  }
}

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
  this->texturesAnya.push_back(
      std::make_shared<Texture>("assets/textures/baloon.png"));
  this->texturesAnya.push_back(
      std::make_shared<Texture>("assets/textures/kitten.png"));
  this->texturesAnya.push_back(
      std::make_shared<Texture>("assets/textures/bomb-anya.png"));
  this->tilemap = std::make_unique<Tilemap>("assets/tilemaps/demo.tmx");

  this->spritesheet =
      std::make_shared<SpriteSheet>("assets/textures/spritesheet.atlas");

  this->font = std::make_unique<Font>("assets/fonts/Vera.ttf", 32);

  this->music = std::make_unique<Music>("assets/music/Pleasant_Creek_Loop.ogg");
  this->soundEffect = std::make_unique<SoundEffect>("assets/sfx/meow.ogg");

  this->music->play_on_loop();
  this->mixer->ToggleMute();

  const glm::vec4 playerRect = this->spritesheet->GetAtlasRect(0);

  // prefabs
  const auto Tink =
      world.prefab("Tink")
          .set<Transform2D>(Transform2D(glm::vec2(80, 400), glm::vec2(1, 1), 0))
          .set<AnimatedSprite>(AnimatedSprite(
              this->spritesheet, this->spritesheet->GetAnimation("Idle")))
          .set<Player>({"Player 1", false})
          .set<Velocity>({glm::vec2(0, 0)})
          .set<CollisionVolume>({glm::vec4(3, 7, 39, 39)})
          .set<Groundable>({false})
          .add<PhysicsBody>();

  const auto Anya = world.prefab("Anya")
                        .set<Transform2D>(Transform2D(glm::vec2(150, 454),
                                                      glm::vec2(1, 1), 0))
                        .set<Sprite>({this->texturesAnya[0]})
                        .set<Velocity>({glm::vec2(0, 0)})
                        .set<CollisionVolume>({
                            glm::vec4(17, 7, 46, 57),
                        })
                        .set<Health>({1.0f})
                        .add<StaticBody>()
                        .set<Groundable>({false});

  const auto HpBar = world.prefab("UIFilledRect")
                         .set<Transform2D>(Transform2D(glm::vec2(0.0f, -15.0f),
                                                       glm::vec2(1, 1), 0))
                         .set<UIFilledRect>(UIFilledRect(
                             glm::vec2(50.0f, 5.0f), 1.0f, 1.0f,
                             glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, 0, 0.3f)));

  // create anya
  // add 1 anya per 64 units on x
  std::string anya_name = "anya";
  for (int i = 0; i < 10; i++) {
    world.entity((anya_name + std::to_string(i)).c_str())
        .is_a(Anya)
        .set<Transform2D>(
            Transform2D(glm::vec2(150 + (64 * i), 454), glm::vec2(1, 1), 0))
        .set<Sprite>({this->texturesAnya[i % 3]});
  }

  // create tink
  const auto player = world.entity("player").is_a(Tink);
  const auto hpBar = world.entity("hpBar").is_a(HpBar).child_of(player);

  // add hurtbox as a child of tink
  const auto hurtbox =
      world.entity("hurtbox")
          .child_of(player)
          .set<Transform2D>(Transform2D(glm::vec2(30, 0), glm::vec2(1, 1), 0))
          .set<CollisionVolume>({glm::vec4(3, 7, 30, 48)})
          .set<Hurtbox>({1.0f, false});

  world.set<Camera>({.position = glm::vec2(0, 0)});
  world.set<Gravity>({.value = 980.0f});

  // player movement + update camera
  world
      .system<Player, Velocity, CollisionVolume, AnimatedSprite, Transform2D,
              Groundable>()
      .iter([](flecs::iter it, Player *p, Velocity *v, CollisionVolume *c,
               AnimatedSprite *s, Transform2D *t, Groundable *g) {
        const float speed = 200.0f;
        const float move_x = InputManager::GetAxisHorizontalMovement();
        const auto jump = InputManager::GetTriggerJump();
        const auto attack =
            InputManager::GetKey(SDL_SCANCODE_LCTRL).IsJustPressed();

        for (int i : it) {
          if (!s[i].isAnimationFinished && !s[i].currentAnimation->loop) {
            return; // the attack animation drives velocity for now
          }

          p->isAttacking = false;

          const auto h = it.entity(0).lookup("hurtbox");
          auto *hurtbox = h.get_mut<Hurtbox>();
          hurtbox->active = false;

          const auto last_velocity = v[i].value;
          v[i].value.x = move_x * speed;
          if (g[i].isGrounded && jump) {
            v[i].value.y = -515.0f;
            g[i].isGrounded = false;
          }

          if (move_x != 0) {
            if (move_x > 0) {
              t[i].scale.x = -1 * fabs(t[i].scale.x);
            } else {
              t[i].scale.x = fabs(t[i].scale.x);
            }
          }

          if (g[i].isGrounded && attack) {
            p->isAttacking = true;
            s[i].SetAnimation(game->spritesheet->GetAnimation("Attack"));
            // play sfx
            game->soundEffect->play();
            const float attack_x_vel = 215.0f;
            if (t[i].scale.x > 0) {
              v[i].value.x = -1 * attack_x_vel;
            } else {
              v[i].value.x = attack_x_vel;
            }
            v[i].value.y = -315.0f;
            g[i].isGrounded = false;
            hurtbox->active = true;
            return;
          }

          // update animations
          if (!g[i].isGrounded) {
            s[i].SetAnimation(game->spritesheet->GetAnimation("Jump"));
          } else if (move_x != 0) {
            s[i].SetAnimation(game->spritesheet->GetAnimation("Run"));
          } else {
            s[i].SetAnimation(game->spritesheet->GetAnimation("Idle"));
          }
        }
      });

  // gravity system
  world.system<Velocity, Groundable>().iter(
      [](flecs::iter it, Velocity *v, Groundable *g) {
        const auto dt = it.delta_time();
        const float grav = game->world.get<Gravity>()->value;
        for (int i : it) {
          if (g[i].isGrounded) {
            v[i].value.y = 0.0f;
          } else {
            v[i].value.y += grav * dt;
          }
        }
      });

  // velocity system
  world.system<Velocity, Transform2D>().iter(
      [](flecs::iter it, Velocity *v, Transform2D *t) {
        const auto dt = it.delta_time();
        for (int i : it) {
          t[i].position += v[i].value * dt;
          t[i].global_position = t[i].position;
        }
      });

  // collision for entities with tilemap
  world.system<Transform2D, CollisionVolume, Groundable>().each(
      [](flecs::entity e, Transform2D &t, CollisionVolume &c, Groundable &g) {
        g.isGrounded = false;
        const auto tilemapBounds = game->tilemap->GetBounds();
        // clamp x position to tilemap bounds
        t.position.x = glm::clamp(
            static_cast<int>(t.position.x),
            static_cast<int>(tilemapBounds.x - c.vertices.x),
            static_cast<int>(game->tilemap->GetBounds().w - c.vertices.z));

        // collide with tiles
        SDL_Rect rect = {static_cast<int>(t.position.x + c.vertices.x),
                         static_cast<int>(t.position.y + c.vertices.y),
                         static_cast<int>(c.vertices.z - c.vertices.x),
                         static_cast<int>(c.vertices.w - c.vertices.y)};
        SDL_Rect found = {0, 0, 0, 0};
        bool isOverlapping = false;
        game->tilemap->IsCollidingWith(&rect, found, e,
                                       g.isGrounded); // check for collision

        if ((found.x != 0 || found.y != 0 || found.w != 0 || found.h != 0)) {
          game->push_rect_transform(rect, found, t, c);
        }
      });

  const auto collisionQuery = game->world.query<Transform2D, CollisionVolume>();
  world.system<Transform2D, CollisionVolume>().each(
      [collisionQuery](flecs::entity e1, Transform2D &t1, CollisionVolume &c1) {
        collisionQuery.each([&e1, &t1, &c1](flecs::entity e2, Transform2D &t2,
                                            CollisionVolume &c2) {
          if (e1.id() == e2.id()) {
            return;
          }
          const SDL_Rect rect1 = {
              static_cast<int>(t1.global_position.x + c1.vertices.x),
              static_cast<int>(t1.global_position.y + c1.vertices.y),
              static_cast<int>(c1.vertices.z - c1.vertices.x),
              static_cast<int>(c1.vertices.w - c1.vertices.y)};
          const SDL_Rect rect2 = {
              static_cast<int>(t2.global_position.x + c2.vertices.x),
              static_cast<int>(t2.global_position.y + c2.vertices.y),
              static_cast<int>(c2.vertices.z - c2.vertices.x),
              static_cast<int>(c2.vertices.w - c2.vertices.y)};

          if (SDL_HasIntersection(&rect1, &rect2)) {
            // apply damage
            if (e1.has<Hurtbox>() && e2.has<Health>() &&
                e1.get<Hurtbox>()->active) {
              e2.get_mut<Health>()->value -= e1.get<Hurtbox>()->damage;
              if (e2.get<Health>()->value <= 0) {
                e2.destruct();
              }
            }
            // push entities apart
            if (e1.has<PhysicsBody>() && e2.has<StaticBody>()) {
              game->push_rect_transform(rect1, rect2, t1, c1);
            }
          }

          // set grounded state
          if (e1.has<Groundable>() && e1.has<Velocity>() &&
              e1.get<Velocity>()->value.y >= 0) {
            auto *g1 = e1.get_mut<Groundable>();
            const SDL_Rect rect3 = {rect2.x + 3, rect2.y - 1, rect2.w - 7, 1};
            if (SDL_HasIntersection(&rect1, &rect3)) {
              g1->isGrounded = true;
            }
          }
        });
      });

  // update sprite batcher uniforms from camera and draw tilemap
  const auto playerQuery =
      game->world.query<Transform2D, Player>(); // note this has to be declared
                                                // outside for emscripten

  world.system<Camera>().each([playerQuery, playerRect](Camera &c) {
    playerQuery.each([&c, playerRect](Transform2D &t, Player &p) {
      glm::vec2 offset = glm::vec2(playerRect.z, playerRect.w) / 2.0f;
      c.position = t.position + offset;
    });
    game->spriteBatcher->UpdateCamera(c.position, game->tilemap->GetBounds());

    // silly but the tilemap needs to be drawn after the camera is updated
    // and before everything else to avoid jitter
    game->tilemap->Draw(game->spriteBatcher.get()); // draw the tilemap
  });

  // update global positions for children
  world.system<Transform2D>().each([](flecs::entity e, Transform2D &t) {
    const auto parent = e.parent();
    if (parent) {
      const auto parent_t = parent.get<Transform2D>();
      t.global_position = parent_t->global_position;
      t.global_position.x += parent_t->scale.x * -t.position.x;
      t.global_position.y += parent_t->scale.y * t.position.y;
    } else {
      t.global_position = t.position;
    }
  });

  // render sprites
  this->world.system<Transform2D, Sprite>().each([](Transform2D &t, Sprite &s) {
    game->spriteBatcher->Draw(s.texture.get(), t.global_position, t.scale,
                              t.rotation);
  });

  // render animated sprites
  // @TODO: generalize this more,
  // right now we are just forcing an animation for a specific sheet
  this->world.system<Transform2D, AnimatedSprite>().each(
      [](Transform2D &t, AnimatedSprite &s) {
        s.currentTime += game->world.delta_time();
        if (!s.isAnimationFinished &&
            s.currentTime >= s.currentAnimation->frameTime) {
          s.currentTime -= s.currentAnimation->frameTime;
          s.currentFrame++;
          if (s.currentFrame >= s.currentAnimation->frames.size()) {
            if (!s.currentAnimation->loop) {
              s.isAnimationFinished = true;
              s.currentFrame--;
              return;
            }
            s.currentFrame = 0;
          }
        }
        game->spriteBatcher->Draw(
            s.spriteSheet->GetTexture(), t.global_position, t.scale, t.rotation,
            glm::vec4(1, 1, 1, 1),
            s.spriteSheet->GetAnimationRect(s.currentAnimation, s.currentFrame),
            s.currentAnimation->dimensions);
      });

  // render health bars
  this->world.system<Transform2D, UIFilledRect>().each(
      [](Transform2D &t, UIFilledRect &u) {
        game->spriteBatcher->DrawRect(
            glm::vec4(t.global_position.x - u.outline_thickness,
                      t.global_position.y - u.outline_thickness,
                      u.dimensions.x + u.outline_thickness * 2,
                      u.dimensions.y + u.outline_thickness * 2),
            u.bg_color);
        game->spriteBatcher->DrawRect(
            glm::vec4(t.global_position.x, t.global_position.y,
                      u.percent * u.dimensions.x, u.dimensions.y),
            u.fill_color);
      });

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

  this->world.progress();

  if (this->drawColliders) {
    // Draw colliders
    world.query<Transform2D, CollisionVolume>().each(
        [](flecs::entity e, Transform2D &t, CollisionVolume &c) {
          // the rect is the vertices with the position offset
          const auto rect =
              c.vertices + glm::vec4(t.global_position.x, t.global_position.y,
                                     -c.vertices.x, -c.vertices.y);
          const bool isGrounded =
              e.has<Groundable>() && e.get<Groundable>()->isGrounded;

          auto color = isGrounded ? glm::vec4(1, 0, 0, 0.5f)
                                  : (game->tilemap->HasCollision(e)
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
        });
    this->tilemap->DrawColliders(this->spriteBatcher.get());
  }

  this->spriteBatcher->Flush();

  this->font->RenderText(this->spriteBatcher.get(),
                         InputManager::GetTextInputBuffer(), glm::vec2(0, 0),
                         glm::vec2(1, 1), glm::vec4(1, 0, 1, 1));

  // draw all sprites in the batch
  this->spriteBatcher->Flush();

  return 0;
}

int Game::unload() { return 0; }

int Game::close() {
  // clean up gl stuff
  return 0;
}
