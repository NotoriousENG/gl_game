#pragma once

#include "components.hpp"
#include "flecs.h"
#include "plugins/enemy.hpp"
#include <glm/glm.hpp>

void SpawnPlayer(flecs::world &ecs, glm::vec2 pos);

void SpawnAnya(flecs::world &ecs, glm::vec2 pos, Path path);