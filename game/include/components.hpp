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

struct Hurtbox {
  float damage;
  bool active;
};

struct Health {
  float value;
};

struct Path {
  std::vector<glm::vec2> points;
  int targetPointIndex;

  Path() : targetPointIndex(0) {}

  Path(std::vector<glm::vec2> points, int targetPointIndex)
      : points(points), targetPointIndex(0) {}
};
