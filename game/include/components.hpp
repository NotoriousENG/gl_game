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

  Transform2D(glm::vec2 position, glm::vec2 scale, float rotation)
      : position(position), scale(scale), rotation(rotation) {}

  Transform2D()
      : position(glm::vec2(0, 0)), scale(glm::vec2(1, 1)), rotation(0) {}

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
  float frameTime;
  float currentTime;
  int currentFrame;

  AnimatedSprite(std::shared_ptr<SpriteSheet> spriteSheet, float frameTime)
      : spriteSheet(spriteSheet), frameTime(frameTime), currentTime(0),
        currentFrame(0) {}
  AnimatedSprite()
      : spriteSheet(nullptr), frameTime(0), currentTime(0), currentFrame(0) {}
};

struct Player {
  std::string name;
};

struct Camera {
  glm::vec2 position;
};

enum class ColliderType { TRIGGER, SOLID };

struct Collider {
  // collider vertices
  glm::vec4 vertices;
  ColliderType type;
  bool isGrounded;
};

struct Velocity {
  glm::vec2 value;
};

struct Gravity {
  float value;
};
