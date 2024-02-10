#pragma once

#include <components.hpp>
#include <font.hpp>
#include <plugins/plugin.hpp>
#include <sprite-batch.hpp>
#include <spritesheet.hpp>
#include <texture.hpp>

// components:

struct Renderer {
  SpriteBatch *renderer;
};

struct Sprite {
  std::shared_ptr<Texture> texture;
};

struct AnimatedSprite {
  std::shared_ptr<SpriteSheet> spriteSheet;
  float currentTime;
  int currentFrame;
  SpriteAnimation *currentAnimation;
  bool isAnimationFinished;

  void SetAnimation(SpriteAnimation *animation) {
    if (this->currentAnimation == animation) {
      return;
    }
    this->currentAnimation = animation;
    this->currentFrame = 0;
    this->currentTime = 0;
    this->isAnimationFinished = false;
  };

  AnimatedSprite(std::shared_ptr<SpriteSheet> spriteSheet,
                 SpriteAnimation *animation)
      : spriteSheet(spriteSheet), currentTime(0), currentFrame(0),
        currentAnimation(animation), isAnimationFinished(false) {}

  AnimatedSprite()
      : spriteSheet(nullptr), currentTime(0), currentFrame(0),
        currentAnimation(nullptr) {}
};

struct UIFilledRect {
  glm::vec2 dimensions;
  float outline_thickness;
  float percent;
  glm::vec4 fill_color;
  glm::vec4 bg_color;

  UIFilledRect()
      : dimensions(glm::vec2(0, 0)), outline_thickness(0), percent(0),
        fill_color(glm::vec4(0, 0, 0, 0)), bg_color(glm::vec4(0, 0, 0, 0)) {}

  UIFilledRect(glm::vec2 dimensions, float outline_thickness, float percent,
               glm::vec4 fill_color, glm::vec4 bg_color)
      : dimensions(dimensions), outline_thickness(outline_thickness),
        percent(percent), fill_color(fill_color), bg_color(bg_color) {}
};

struct AdjustingTextBox {
  std::shared_ptr<Font> font;
  const char *text;
  float t;
  float duration;

  AdjustingTextBox() : font(nullptr), text(""), t(0), duration(0) {}

  AdjustingTextBox(std::shared_ptr<Font> font, const char *text, float duration)
      : font(font), text(text), t(0), duration(duration) {}
};

// systems:

void renderSprite(SpriteBatch *renderer, Transform2D &t, Sprite &s);

void renderAnimatedSprite(SpriteBatch *renderer, float delta, Transform2D &t,
                          AnimatedSprite &s);

void renderUIFilledRect(SpriteBatch *renderer, Transform2D &t, UIFilledRect &r);

void renderAdjustingTextBox(SpriteBatch *renderer, Transform2D &t,
                            UIFilledRect &u, AdjustingTextBox &b);

// plugin:
class GraphicsPlugin : public Plugin {
public:
  void addSystems(flecs::world &ecs) override;
};