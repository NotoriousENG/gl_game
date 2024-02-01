#include "plugins/player.hpp"

void playerUpdate(flecs::iter it, Player *p, Velocity *v, CollisionVolume *c,
                  AnimatedSprite *s, Transform2D *t, Groundable *g) {
  const float speed = 200.0f;
  const auto move = InputManager::GetVectorMovement();
  const auto jump = InputManager::GetTriggerJump();
  const auto attack = InputManager::GetKey(SDL_SCANCODE_LCTRL).IsJustPressed();
  const auto fire = InputManager::GetKey(SDL_SCANCODE_Z).IsJustPressed();

  for (int i : it) {
    if (!s[i].isAnimationFinished && !s[i].currentAnimation->loop) {
      return; // the attack animation drives velocity for now
    }

    p->isAttacking = false;

    const auto h = it.entity(0).lookup("hurtbox");
    auto *hurtbox = h.get_mut<Hurtbox>();
    hurtbox->active = false;

    const auto last_velocity = v[i].value;
    v[i].value.x = move.x * speed;
    if (g[i].isGrounded && jump) {
      v[i].value.y = -515.0f;
      g[i].isGrounded = false;
    }

    const auto dir = it.entity(0).lookup("directionArrow");
    auto *dir_t = dir.get_mut<Transform2D>();
    if (move.x != 0.0f || move.y != 0.0f) {
      dir_t->rotation = atan2(-move.y, move.x);
    }

    if (move.x != 0.0f) {
      if (move.x > 0.0f) {
        t[i].scale.x = -1 * fabs(t[i].scale.x);
      } else {
        t[i].scale.x = fabs(t[i].scale.x);
      }
    }

    if (g[i].isGrounded && attack) {
      p->isAttacking = true;
      s[i].SetAnimation(s->spriteSheet->GetAnimation("Attack"));
      // play sfx
      p->soundEffect->play();
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
    if (fire) {
      const auto world = it.world();
      const auto b = world.lookup("Ball");
      const auto base_up_velocity = glm::vec2(0, -200.0f);
      const auto ball_velocity =
          glm::vec2(cos(dir_t->rotation), -sin(dir_t->rotation)) * 800.0f +
          base_up_velocity;
      const auto ball =
          world.entity()
              .is_a(b)
              .set<Transform2D>(Transform2D().WithPosition(t[i].position))
              .set<Velocity>({ball_velocity})
              .set<Groundable>({false})
              .set<LiveFor>({5.0f});
      SDL_Log("fin ball");
    }

    // update animations
    if (!g[i].isGrounded) {
      s[i].SetAnimation(s->spriteSheet->GetAnimation("Jump"));
    } else if (move.x != 0.0f) {
      s[i].SetAnimation(s->spriteSheet->GetAnimation("Run"));
    } else {
      s[i].SetAnimation(s->spriteSheet->GetAnimation("Idle"));
    }
  }
}

void PlayerPlugin::addSystems(flecs::world &ecs) {
  ecs.system<Player, Velocity, CollisionVolume, AnimatedSprite, Transform2D,
             Groundable>()
      .iter([](flecs::iter it, Player *p, Velocity *v, CollisionVolume *c,
               AnimatedSprite *s, Transform2D *t,
               Groundable *g) { playerUpdate(it, p, v, c, s, t, g); });
}
