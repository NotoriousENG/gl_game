#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "texture.hpp"

struct SpriteAnimation {
  std::vector<int> frames;
  float frameTime;
  glm::vec2 dimensions;
  bool loop;
  SpriteAnimation(std::vector<int> frames, float frameTime,
                  glm::vec2 dimensions, bool loop)
      : frames(frames), frameTime(frameTime), dimensions(dimensions),
        loop(loop) {}
  SpriteAnimation() : frames({0}), frameTime(0.0f), dimensions(glm::vec2(0)) {}
};

class SpriteSheet {
public:
  SpriteSheet(const char *atlasPath);
  ~SpriteSheet();

  Texture *GetTexture();

  const glm::vec4 GetAtlasRect(size_t index);

  const size_t GetSpriteCount();

  SpriteAnimation *GetAnimation(const char *name);

  glm::vec4 GetAnimationRect(const SpriteAnimation *animation, size_t index);

private:
  struct SpriteAtlas {
    std::string texturePath;
    std::unordered_map<std::string, SpriteAnimation> animations;
    std::vector<glm::vec4> atlas;
  };

  void loadAtlas(const char *atlasPath);

  std::shared_ptr<Texture> texture;
  std::vector<int> atlas;
  size_t numRects;
  std::unordered_map<std::string, SpriteAnimation> animations;
};
