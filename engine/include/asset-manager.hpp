#pragma once

#include "font.hpp"
#include <SDL2/SDL.h>
#include <memory>
#include <unordered_map>
#include <vector>

// Lazy-loaded asset manager, to minimize loading during transitions, ensure
// there is at least one shared_ptr in scope
template <class T> class AssetManager {
private:
  std::unordered_map<std::string, std::weak_ptr<T>> assets;
  inline const static std::unique_ptr<AssetManager<T>> instance =
      std::make_unique<AssetManager<T>>();

  inline static std::vector<std::shared_ptr<T>> lockedAssets;

public:
  static std::shared_ptr<T> get(std::string path) {
    auto asset = instance->assets.find(path);
    if (asset != instance->assets.end() && !asset->second.expired()) {
      SDL_Log("Cache Hit: %s", path.c_str());
      return asset->second.lock();
    } else {
      SDL_Log("Cache Miss: %s", path.c_str());
      std::shared_ptr<T> newAsset = std::make_shared<T>(path.c_str());
      instance->assets[path] = newAsset;
      return newAsset;
    }
  }
  // temporarily lock all assets to prevent them from being unloaded
  static void lockAll() {
    for (auto &asset : instance->assets) {
      if (!asset.second.expired()) {
        instance->lockedAssets.push_back(asset.second.lock());
      }
    }
  }
  // unlock all assets to allow them to be unloaded
  static void unlockAll() { instance->lockedAssets.clear(); }

  static std::shared_ptr<Font> getFont(std::string path, int size) {
    const auto id = path + "?" + std::to_string(size);
    auto asset = instance->assets.find(id);
    if (asset != instance->assets.end() && !asset->second.expired()) {
      SDL_Log("Cache Hit: %s", id.c_str());
      return asset->second.lock();
    } else {
      SDL_Log("Cache Miss: %s", id.c_str());
      std::shared_ptr<T> newAsset = std::make_shared<Font>(path.c_str(), size);
      instance->assets[id] = newAsset;
      return newAsset;
    }
  }
};