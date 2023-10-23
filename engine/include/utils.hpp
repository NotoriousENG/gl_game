#pragma once
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

static glm::vec4 sdlRectToVec4(const SDL_Rect &rect) {
  return glm::vec4(rect.x, rect.y, rect.w, rect.h);
}