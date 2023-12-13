#pragma once
#include <flecs.h>
#include <glm/glm.hpp>
#include <memory>
#include <spritesheet.hpp>
#include <string>
#include <texture.hpp>

struct Transform2D {
  glm::vec2 position;
  glm::vec2 scale;
  float rotation;
  glm::vec2 global_position;

  Transform2D(glm::vec2 position, glm::vec2 scale, float rotation)
      : position(position), scale(scale), rotation(rotation),
        global_position(position) {}

  Transform2D()
      : position(glm::vec2(0, 0)), scale(glm::vec2(1, 1)), rotation(0),
        global_position(position) {}

  Transform2D WithPosition(glm::vec2 position) {
    this->position = position;
    return *this;
  }

  Transform2D WithScale(glm::vec2 scale) {
    this->scale = scale;
    return *this;
  }

  Transform2D WithRotation(float rotation) {
    this->rotation = rotation;
    return *this;
  }
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

struct Player {
  std::string name;
  bool isAttacking; // @TODO move to child collider
};

struct Camera {
  glm::vec2 position;
};

struct Groundable {
  bool isGrounded;
};

struct CollisionVolume {
  glm::vec4 vertices;
};

struct PhysicsBody {};

struct StaticBody {};

struct Hurtbox {
  float damage;
  bool active;
};

struct Health {
  float value;
};

struct Velocity {
  glm::vec2 value;
};

struct Gravity {
  float value;
};

struct Path {
  std::vector<glm::vec2> points;
  int targetPointIndex;

  Path() : targetPointIndex(0) {}

  Path(std::vector<glm::vec2> points, int targetPointIndex)
      : points(points), targetPointIndex(0) {}
};
struct Enemy {
  bool seesPlayer;
  Enemy() : seesPlayer(false) {}
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
