#include "asset-manager-aggregates.hpp"
#include <asset-manager.hpp>
#include <font.hpp>
#include <mixer.hpp>
#include <spritesheet.hpp>
#include <texture.hpp>

void LockAllAssets() {
  AssetManager<Texture>::lockAll();
  AssetManager<Font>::lockAll();
  AssetManager<Music>::lockAll();
  AssetManager<SoundEffect>::lockAll();
  AssetManager<SpriteSheet>::lockAll();
}

void UnlockAllAssets() {
  AssetManager<Texture>::unlockAll();
  AssetManager<Font>::unlockAll();
  AssetManager<Music>::unlockAll();
  AssetManager<SoundEffect>::unlockAll();
  AssetManager<SpriteSheet>::unlockAll();
}
