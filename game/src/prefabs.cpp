#include "prefabs.hpp"

#include "asset-manager.hpp"
#include "plugins/enemy.hpp"
#include "plugins/graphics.hpp"
#include "plugins/physics.hpp"
#include "plugins/player.hpp"
#include "resource-paths.hpp"
#include <mixer.hpp>

void SpawnPlayer(flecs::world &ecs, glm::vec2 pos) {

  const auto spritesheet = AssetManager<SpriteSheet>::get(RES_SHEET_PLAYER);
  const auto music = AssetManager<Music>::get(RES_MUSIC_PLEASANT_CREEK);
  const auto soundEffect = AssetManager<SoundEffect>::get(RES_SFX_MEOW);
  const auto textureArrow = AssetManager<Texture>::get(RES_TEXTURE_ARROW);
  const auto textureBall = AssetManager<Texture>::get(RES_TEXTURE_BALL);
  const auto fontS = AssetManager<Font>::getFont(RES_FONT_VERA, 14);

  music->play_on_loop();

  const auto Tink =
      ecs.prefab("Tink")
          .set<Transform2D>(Transform2D(glm::vec2(80, 400), glm::vec2(1, 1), 0))
          .set<AnimatedSprite>(
              AnimatedSprite(spritesheet, spritesheet->GetAnimation("Idle")))
          .set<Player>({"Player 1", false, soundEffect, music,
                        spritesheet->GetAtlasRect(0)})
          .set<Velocity>({glm::vec2(0, 0)})
          .set<CollisionVolume>({glm::vec4(3, 7, 39, 39)})
          .set<Groundable>({false})
          .add<PhysicsBody>();

  const auto Ball =
      ecs.prefab("Ball")
          .set<Transform2D>(Transform2D(glm::vec2(0, 0), glm::vec2(1, 1), 0))
          .set<Sprite>({textureBall})
          .set<Velocity>({glm::vec2(0, 0)})
          .set<Groundable>({false})
          .set<CollisionVolume>({
              glm::vec4(0, 0, 16, 16),
          })
          .set<LiveFor>({5.0f});

  const auto HpBar = ecs.prefab("UIFilledRect")
                         .set<Transform2D>(Transform2D(glm::vec2(0.0f, -15.0f),
                                                       glm::vec2(1, 1), 0))
                         .set<UIFilledRect>(UIFilledRect(
                             glm::vec2(50.0f, 5.0f), 1.0f, 1.0f,
                             glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, 0, 0.3f)));

  const auto DirectionArrow =
      ecs.prefab("DirectionIndicator")
          .set<Transform2D>(
              Transform2D(glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), 0))
          .set<Sprite>({textureArrow});

  const auto TextArea =
      ecs.prefab("textArea")
          .set<Transform2D>(
              Transform2D(glm::vec2(-45.0f, -200.0), glm::vec2(1, 1), 0))
          .set<UIFilledRect>(UIFilledRect(glm::vec2(128.0f, 128.0f), 1.0f, 1.0f,
                                          glm::vec4(1, 1, 1, 1),
                                          glm::vec4(0, 0, 0, 0.3f)))
          .set<AdjustingTextBox>(
              AdjustingTextBox(fontS, InputManager::GetTextInputBuffer(), 0));

  // create tink
  const auto player = ecs.entity("player").is_a(Tink);
  const auto hpBar = ecs.entity("hpBar").is_a(HpBar).child_of(player);
  const auto directionArrow =
      ecs.entity("directionArrow").is_a(DirectionArrow).child_of(player);
  const auto textArea =
      ecs.entity("playerChatbox").is_a(TextArea).child_of(player);

  // add hurtbox as a child of tink
  const auto hurtbox =
      ecs.entity("hurtbox")
          .child_of(player)
          .set<Transform2D>(Transform2D(pos, glm::vec2(1, 1), 0))
          .set<CollisionVolume>({glm::vec4(3, 7, 30, 48)})
          .set<Hurtbox>({1.0f, false});
}

void SpawnAnya(flecs::world &ecs, glm::vec2 pos, Path path) {
  const auto textureAnya = AssetManager<Texture>::get(RES_TEXTURE_AMIIBO);
  const auto Anya = ecs.entity()
                        .set<Transform2D>(Transform2D(pos, glm::vec2(1, 1), 0))
                        .set<Sprite>({textureAnya})
                        .set<Velocity>({glm::vec2(0, 0)})
                        .set<CollisionVolume>({
                            glm::vec4(0, 32, 64, 64),
                        })
                        .set<Health>({1.0f})
                        .set<Enemy>(Enemy())
                        .set<Path>(path);
}
