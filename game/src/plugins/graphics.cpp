#include "plugins/graphics.hpp"

void renderSprite(SpriteBatch *renderer, Transform2D &t, Sprite &s) {
  renderer->Draw(s.texture.get(), t.global_position, t.scale, t.rotation);
}

void renderAnimatedSprite(SpriteBatch *renderer, float delta, Transform2D &t,
                          AnimatedSprite &s) {
  s.currentTime += delta;
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
  renderer->Draw(
      s.spriteSheet->GetTexture(), t.global_position, t.scale, t.rotation,
      glm::vec4(1, 1, 1, 1),
      s.spriteSheet->GetAnimationRect(s.currentAnimation, s.currentFrame),
      s.currentAnimation->dimensions);
}

void renderUIFilledRect(SpriteBatch *renderer, Transform2D &t,
                        UIFilledRect &u) {
  renderer->DrawRect(glm::vec4(t.global_position.x - u.outline_thickness,
                               t.global_position.y - u.outline_thickness,
                               u.dimensions.x + u.outline_thickness * 2,
                               u.dimensions.y + u.outline_thickness * 2),
                     u.bg_color);
  renderer->DrawRect(glm::vec4(t.global_position.x, t.global_position.y,
                               u.percent * u.dimensions.x, u.dimensions.y),
                     u.fill_color);
}

void renderAdjustingTextBox(SpriteBatch *renderer, Transform2D &t,
                            UIFilledRect &u, AdjustingTextBox &b) {
  // the max width is 128
  const auto max_width = 128.0f;

  const int fontSize = b.font->GetFontSize();
  const int rowSpacing = fontSize * 0.75f;

  // set the position of the rendered text
  const auto tPos = glm::vec2(t.global_position.x + rowSpacing / 2,
                              t.global_position.y + rowSpacing / 2);

  b.font->RenderText(renderer, b.text, tPos, t.scale, glm::vec4(0, 0, 0, 1),
                     &u.dimensions, max_width);

  // change the transform offset to be -36 - the height of the text
  t.position.y = -36 - u.dimensions.y;
}

void GraphicsPlugin::addSystems(flecs::world &ecs) {
  SpriteBatch *r = ecs.get<Renderer>()->renderer;

  ecs.system<Transform2D, AnimatedSprite>().iter(
      [r](flecs::iter it, Transform2D *t, AnimatedSprite *s) {
        for (int i : it) {
          renderAnimatedSprite(r, it.delta_time(), t[i], s[i]);
        }
      });

  ecs.system<Transform2D, Sprite>().each(
      [r](Transform2D &t, Sprite &s) { renderSprite(r, t, s); });

  ecs.system<Transform2D, UIFilledRect>().each(
      [r](Transform2D &t, UIFilledRect &u) { renderUIFilledRect(r, t, u); });

  ecs.system<Transform2D, UIFilledRect, AdjustingTextBox>().iter(
      [r](flecs::iter it, Transform2D *t, UIFilledRect *u,
          AdjustingTextBox *b) {
        r->Flush();
        for (int i : it) {
          renderAdjustingTextBox(r, t[i], u[i], b[i]);
        }
        r->Flush();
      });
}
