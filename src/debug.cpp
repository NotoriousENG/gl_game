#include "debug.hpp"
#include <memory>

static std::unique_ptr<DebugManager> instance =
    std::make_unique<DebugManager>();

void DebugManager::ToggleRenderColliders() {
  std::lock_guard<std::mutex> lock(instance->mtx);
  instance->render_colliders = !instance->render_colliders;
}

bool DebugManager::GetRenderColliders() {
  std::lock_guard<std::mutex> lock(instance->mtx);
  return instance->render_colliders;
}
