#pragma once

#include "components.hpp"
#include "flecs.h"
#include <glm/glm.hpp>

void SpawnPlayer(flecs::world &ecs, glm::vec2 pos);

void SpawnAnya(flecs::world &ecs, glm::vec2 pos);