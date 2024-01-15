#include "font.hpp"
#include <SDL.h>

Font::Font(const char *path, int size) {

  this->fontSize = size;

  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loading font %s %i", path,
              this->fontSize);

  // @Todo, we should keep this arround for additional fonts refactor later
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
  if (FT_Set_Pixel_Sizes(face, 0, this->fontSize)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not set font size");
    return;
  }

  const int padding = 2;
  int row = 0;
  int col = padding;

  char texData[this->texDim * this->texDim * 4];
  memset(texData, 0, sizeof(texData)); // VLAs can't be initialized in em++ :/

  // Load glyphs from the ASCII set (ASCII 32 - 126)
  for (FT_ULong ascii = ASCII_OFFSET; ascii <= ASCII_EXTENDED_MAX; ++ascii) {
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

    if (col + face->glyph->bitmap.width + padding >= this->texDim) {
      // Reached bottom of texture, move to next column
      col = padding;
      row += this->fontSize;
    }

    this->max_height = std::max(
        (face->size->metrics.ascender - face->size->metrics.descender) >> 6,
        this->max_height);

    for (uint y = 0; y < face->glyph->bitmap.rows; ++y) {
      for (uint x = 0; x < face->glyph->bitmap.width; ++x) {
        // if we are out of bounds, break
        if (col + x >= this->texDim || row + y >= this->texDim) {
          SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                       "Out of bounds on glyph %c, (%i, %i)", (char)ascii,
                       col + x, row + y);
          break;
        }

        const auto val =
            face->glyph->bitmap.buffer[y * face->glyph->bitmap.width + x];
        // set rgba
        texData[4 * (this->texDim * (row + y) + (col + x)) + 0] = val;
        texData[4 * (this->texDim * (row + y) + (col + x)) + 1] = val;
        texData[4 * (this->texDim * (row + y) + (col + x)) + 2] = val;
        texData[4 * (this->texDim * (row + y) + (col + x)) + 3] = val;
      }
    }

    const Glyph g = {.offset = glm::vec2((float)face->glyph->bitmap_left,
                                         (float)face->glyph->bitmap_top),
                     .size = glm::ivec2((int)face->glyph->bitmap.width,
                                        (int)face->glyph->bitmap.rows),
                     .texCoords = glm::ivec2(col, row),
                     .advance =
                         glm::vec2((float)(face->glyph->advance.x >> 6),
                                   (float)(face->glyph->advance.y >> 6))};

    const auto p = std::make_pair((char)ascii, g);

    this->glyphs.insert(p);

    col += face->glyph->bitmap.width + padding;
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

  // Set texture options
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // @TODO: make shader work with red on black
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->texDim, this->texDim, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, (char *)texData);
}

void Font::RenderText(SpriteBatch *renderer, const char *text,
                      glm::vec2 position, glm::vec2 scale, glm::vec4 color,
                      glm::vec2 *outDims, float wrapWidth) {
  glBindTexture(GL_TEXTURE_2D, this->tex);

  renderer->SetTextureAndDimensions(this->tex, this->texDim, this->texDim);

  const auto startY = position.y;
  const auto startX = position.x;

  // add the fontsize * 2 to the y position, the top left is the anchor point
  position.y += this->fontSize;

  float currentWidth = 0;

  for (int i = 0; i < strlen(text); i++) {
    Glyph *g = &this->glyphs.at(text[i]);
    if (g == nullptr) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not find glyph %c",
                   text[i]);
      continue;
    }

    const glm::vec4 srcRect =
        glm::vec4(g->texCoords.x, g->texCoords.y, g->size.x, g->size.y);

    const auto adjustedPosition = glm::vec2(position.x + g->offset.x * scale.x,
                                            position.y - g->offset.y * scale.y);

    renderer->Draw(this->tex, adjustedPosition, scale, 0.0f, color, srcRect);

    position.x += g->advance.x * scale.x;

    currentWidth += g->advance.x * scale.x;

    if (wrapWidth >= 0 && currentWidth > wrapWidth) {
      position.y += this->fontSize * 1.5f;
      position.x = startX;
      currentWidth = 0;
    }
  }

  if (outDims != nullptr) {

    int w;
    if (wrapWidth <= 0) {
      w = currentWidth;
    } else {
      w = wrapWidth;
    }

    // if no text length is 0
    if (strlen(text) == 0) {
      *outDims = glm::vec2(0, 0);
      return;
    }

    *outDims = glm::vec2(w + fontSize, position.y - startY + fontSize);
  }
}

glm::vec2 Font::GetTextDimensions(const char *text) {
  glm::vec2 dimensions = glm::vec2(0, 0);
  for (int i = 0; i < strlen(text); i++) {
    Glyph *g = &this->glyphs.at(text[i]);
    if (g == nullptr) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not find glyph %c",
                   text[i]);
      continue;
    }

    dimensions.x += g->advance.x;
    dimensions.y = std::max(dimensions.y, (float)g->size.y);
  }
  return dimensions;
}
