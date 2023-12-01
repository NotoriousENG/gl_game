#include "font.hpp"
#include <SDL.h>

Font::Font(const char *path, int size) {

  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loading font %s %i", path, size);

  FT_Library ft;
  if (FT_Init_FreeType(&ft)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not init freetype");
    return;
  }

  // Load font as face
  FT_Face face;
  if (FT_New_Face(ft, path, 0, &face)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not open font %s", path);
    return;
  }

  // Set size to load glyphs as
  if (FT_Set_Pixel_Sizes(face, 0, size)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not set font size");
    return;
  }

  const int padding = 2;
  int y = 0;
  int x = padding;

  const int texDim = 512;
  char texData[texDim * texDim];

  // Load glyphs from the ASCII set (ASCII 32 - 126)
  for (FT_ULong ascii = ASCII_OFFSET; ascii <= ASCII_MAX; ++ascii) {
    FT_UInt glyph_index = FT_Get_Char_Index(face, ascii);
    if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT)) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load glyph %c",
                   (char)ascii);
      continue;
    }
    if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not render glyph %c",
                   (char)ascii);
      continue;
    }

    if (y + face->glyph->bitmap.width >= texDim) {
      // Reached bottom of texture, move to next column
      x = padding;
      y += size;
    }

    this->font_height = std::max(
        (face->size->metrics.ascender - face->size->metrics.descender) >> 6,
        font_height);

    for (uint r = 0; r < face->glyph->bitmap.rows; ++r) {
      for (uint c = 0; c < face->glyph->bitmap.width; ++c) {
        texData[(y + r) * texDim + x + c] =
            face->glyph->bitmap.buffer[r * face->glyph->bitmap.width + c];
      }
    }

    int index = ascii - ASCII_OFFSET;
    if (index < 0 || index > ASCII_COUNT) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "Glyph index out of bounds: %i", index);
      return;
    }
    Glyph *g = &this->glyphs[index];
    // @todo get texCoords
    g->offset = glm::vec2((float)face->glyph->bitmap_left,
                          (float)face->glyph->bitmap_top);
    g->size = glm::ivec2((int)face->glyph->bitmap.width,
                         (int)face->glyph->bitmap.rows);
    // g->texCoords = {(int)x, (int)y};
    g->advance = glm::vec2((float)(face->glyph->advance.x >> 6),
                           (float)(face->glyph->advance.y >> 6));

    x += face->glyph->bitmap.rows + padding;
  }

  if (FT_Done_Face(face)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not close font %s", path);
    return;
  }
  if (FT_Done_FreeType(ft)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not close freetype");
    return;
  }

  // Upload texture to GPU
  glGenTextures(1, &this->tex);
  glBindTexture(GL_TEXTURE_2D, this->tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, texDim, texDim, 0, GL_RED,
               GL_UNSIGNED_BYTE, (char *)texData);
}

void Font::RenderText(SpriteBatch *renderer, const char *text,
                      glm::vec2 position, glm::vec2 scale, glm::vec4 color) {
  glBindTexture(GL_TEXTURE_2D, this->tex);
  renderer->SetTextureAndDimensions(this->tex, 512, 512);

  for (int i = 0; i < strlen(text); i++) {
    Glyph *g = &this->glyphs[text[i] - ASCII_OFFSET];

    renderer->DrawRect(glm::vec4(position.x + g->offset.x * scale.x,
                                 position.y + g->offset.y * scale.y,
                                 g->size.x * scale.x, g->size.y * scale.y),
                       color);

    position.x += g->advance.x * scale.x;
  }
}
