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

Game::Game() { game = this; }

Game::~Game() {}

void Game::push_rect_transform(const SDL_Rect &rect, const SDL_Rect &pushedBy,
                               Transform2D &t1, Collider &c1) {
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

  // map the text_input_buffer
  InputManager::SetTextInputBuffer(&shared_data->text_input_buffer[0]);

  // Get current window size
  int w, h;
  SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);
  this->spriteBatcher = std::make_unique<SpriteBatch>(glm::vec2(w, h));
  this->textureTink = std::make_shared<Texture>("assets/textures/tink.png");
  this->textureAnya = std::make_shared<Texture>("assets/textures/anya.png");
  this->tilemap = std::make_unique<Tilemap>("assets/tilemaps/demo.tmx");

  this->spritesheet =
      std::make_shared<SpriteSheet>("assets/textures/spritesheet.atlas");

  this->font = std::make_unique<Font>("assets/fonts/Vera.ttf", 32);

  const glm::vec4 playerRect = this->spritesheet->GetAtlasRect(0);

  // prefabs
  const auto Tink =
      world.prefab("Tink")
          .set<Transform2D>(
              Transform2D(glm::vec2(300, 400), glm::vec2(1, 1), 0))
          .set<AnimatedSprite>(AnimatedSprite(
              this->spritesheet, this->spritesheet->GetAnimation("Idle")))
          .set<Player>({"Player 1"})
          .set<Velocity>({glm::vec2(0, 0)})
          .set<Collider>({glm::vec4(3, 7, 39, 39), ColliderType::SOLID, false});

  const auto Anya = world.prefab("Anya")
                        .set<Transform2D>(Transform2D(glm::vec2(150, 454),
                                                      glm::vec2(1, 1), 0))
                        .set<Sprite>({this->textureAnya})
                        .set<Velocity>({glm::vec2(0, 0)})
                        .set<Collider>({glm::vec4(17, 7, 46, 57),
                                        ColliderType::SOLID, false});

  // create anya
  const auto anya = world.entity("anya").is_a(Anya);

  // create tink
  const auto player = world.entity("player").is_a(Tink);

  world.set<Camera>({.position = glm::vec2(0, 0)});
  world.set<Gravity>({.value = 980.0f});

  // player movement + update camera
  world.system<Player, Velocity, Collider, AnimatedSprite, Transform2D>().iter(
      [](flecs::iter it, Player *p, Velocity *v, Collider *c, AnimatedSprite *s,
         Transform2D *t) {
        const float speed = 200.0f;
        const float move_x = InputManager::GetAxisHorizontalMovement();
        const auto jump = InputManager::GetTriggerJump();
        for (int i : it) {
          const auto last_velocity = v[i].value;
          v[i].value.x = move_x * speed;
          if (c[i].isGrounded && jump) {
            v[i].value.y = -515.0f;
            c[i].isGrounded = false;
          }
          // update animations
          if (!c[i].isGrounded) {
            s[i].SetAnimation(game->spritesheet->GetAnimation("Jump"));
          } else if (move_x != 0) {
            s[i].SetAnimation(game->spritesheet->GetAnimation("Run"));
          } else {
            s[i].SetAnimation(game->spritesheet->GetAnimation("Idle"));
          }
          if (move_x != 0) {
            if (move_x >= 0) {
              t[i].scale.x = -1 * fabs(t[i].scale.x);
            } else {
              t[i].scale.x = fabs(t[i].scale.x);
            }
          }
        }
      });

  // gravity system
  world.system<Velocity, Collider>().iter(
      [](flecs::iter it, Velocity *v, Collider *c) {
        const auto dt = it.delta_time();
        const float g = game->world.get<Gravity>()->value;
        for (int i : it) {
          if (c[i].isGrounded) {
            v[i].value.y = 0.0f;
          } else {
            v[i].value.y += g * dt;
          }
        }
      });

  // velocity system
  world.system<Velocity, Transform2D>().iter(
      [](flecs::iter it, Velocity *v, Transform2D *t) {
        const auto dt = it.delta_time();
        for (int i : it) {
          t[i].position += v[i].value * dt;
        }
      });

  // collision for entities with tilemap
  world.system<Transform2D, Collider>().each(
      [](flecs::entity e, Transform2D &t, Collider &c) {
        c.isGrounded = false; // reset grounded state
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
                                       c.isGrounded); // check for collision

        if ((found.x != 0 || found.y != 0 || found.w != 0 || found.h != 0)) {
          game->push_rect_transform(rect, found, t, c);
        }
      });

  // Colision Between player + entities
  const auto collisionQuery = game->world.query<Transform2D, Collider>();
  world.system<Transform2D, Collider, Player>().each([collisionQuery](
                                                         flecs::entity e1,
                                                         Transform2D &t1,
                                                         Collider &c1,
                                                         Player &p1) {
    collisionQuery.each([&e1, &t1, &c1](flecs::entity e2, Transform2D &t2,
                                        Collider &c2) {
      if (e1.id() == e2.id()) {
        return;
      }
      const SDL_Rect rect1 = {static_cast<int>(t1.position.x + c1.vertices.x),
                              static_cast<int>(t1.position.y + c1.vertices.y),
                              static_cast<int>(c1.vertices.z - c1.vertices.x),
                              static_cast<int>(c1.vertices.w - c1.vertices.y)};
      const SDL_Rect rect2 = {static_cast<int>(t2.position.x + c2.vertices.x),
                              static_cast<int>(t2.position.y + c2.vertices.y),
                              static_cast<int>(c2.vertices.z - c2.vertices.x),
                              static_cast<int>(c2.vertices.w - c2.vertices.y)};

      if (SDL_HasIntersection(&rect1, &rect2)) {
        game->push_rect_transform(rect1, rect2, t1, c1);
      }

      // if there is an intersection with a rect slightly above the second
      // rect then the player is grounded
      const SDL_Rect rect3 = {rect2.x + 3, rect2.y - 1, rect2.w - 7, 1};
      if (SDL_HasIntersection(&rect1, &rect3)) {
        c1.isGrounded = true;
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
  // render sprites
  this->world.system<Transform2D, Sprite>().each([](Transform2D &t, Sprite &s) {
    game->spriteBatcher->Draw(s.texture.get(), t.position, t.scale, t.rotation);
  });

  // render animated sprites
  // @TODO: generalize this more,
  // right now we are just forcing an animation for a specific sheet
  this->world.system<Transform2D, AnimatedSprite>().each(
      [](Transform2D &t, AnimatedSprite &s) {
        s.currentTime += game->world.delta_time();
        if (s.currentTime >= s.currentAnimation->frameTime) {
          s.currentTime -= s.currentAnimation->frameTime;
          s.currentFrame++;
          if (s.currentFrame >= s.currentAnimation->frames.size()) {
            s.currentFrame = 0;
          }
        }
        game->spriteBatcher->Draw(
            s.spriteSheet->GetTexture(), t.position, t.scale, t.rotation,
            glm::vec4(1, 1, 1, 1),
            s.spriteSheet->GetAnimationRect(s.currentAnimation, s.currentFrame),
            s.currentAnimation->dimensions);
      });

  return 0;
}

int Game::update() {
  int num_keys;
  const Uint8 *key_state = SDL_GetKeyboardState(&num_keys);
  InputManager::Update(key_state, num_keys);

  if (InputManager::GetKey(SDL_SCANCODE_1).IsJustPressed()) {
    this->drawColliders = !this->drawColliders;
  };

  if (InputManager::GetKey(SDL_SCANCODE_RETURN).IsJustPressed()) {
    InputManager::ToggleTextInput();
  };

  this->world.progress();

  if (this->drawColliders) {
    // Draw colliders
    world.query<Transform2D, Collider>().each([](flecs::entity e,
                                                 Transform2D &t, Collider &c) {
      // the rect is the vertices with the position offset
      const auto rect = c.vertices + glm::vec4(t.position.x, t.position.y,
                                               -c.vertices.x, -c.vertices.y);
      const auto color = c.isGrounded ? glm::vec4(1, 0, 0, 0.5f)
                                      : (game->tilemap->HasCollision(e)
                                             ? glm::vec4(0, 1, 0, 0.5f)
                                             : glm::vec4(0, 0, 1, 0.5f));
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
