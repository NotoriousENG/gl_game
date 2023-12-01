#pragma once
#include <ft2build.h>

#include "defs.hpp"

#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "sprite-batch.hpp"

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
  long font_height;

  Glyph glyphs[ASCII_COUNT];

  GLuint tex;
};