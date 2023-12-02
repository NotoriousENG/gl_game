#pragma once
#include <ft2build.h>

#include "defs.hpp"

#include FT_FREETYPE_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "sprite-batch.hpp"

#include <unordered_map>

struct Glyph {
  glm::vec2 offset;
  glm::ivec2 size;
  glm::ivec2 texCoords;
  glm::vec2 advance;
};

class Font {
public:
  Font(const char *path, int size);
  void RenderText(SpriteBatch *renderer, const char *text, glm::vec2 position,
                  glm::vec2 scale, glm::vec4 color);

private:
  int fontSize;
  long max_height;

  std::unordered_map<char, Glyph> glyphs;

  GLuint tex;
  const int texDim = 512;
};