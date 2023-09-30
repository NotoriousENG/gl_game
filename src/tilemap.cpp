#include "tilemap.hpp"
#include "SDL2/SDL_log.h"
#include "defs.hpp"
#include "glad/glad.h"

Tilemap::Tilemap(const char *path) {
  this->map.load(path);

  // load the textures
  const auto &tilesets = map.getTilesets();
  for (const auto &ts : tilesets) {
    this->textures.push_back(
        std::make_shared<Texture>(ts.getImagePath().c_str()));
  }
}

Tilemap::~Tilemap() {}

// @TODO: use a pixel buffer and do this on the GPU instead of drawing a bunch
// of quads
void Tilemap::Draw(SpriteBatch *spriteBatch) {
  // loop over the map's layers
  for (int i = 0; i < map.getLayers().size(); i++) {
    const auto &layer = map.getLayers()[i];
    // skip all layers that are not tile layers
    if (layer->getType() != tmx::Layer::Type::Tile) {
      continue;
    }
    // cast to a tile layer
    const auto tileLayer = layer->getLayerAs<tmx::TileLayer>();

    // loop over all the tiles in the layer (x and y)
    for (int x = 0; x < tileLayer.getSize().x; x++) {
      for (int y = 0; y < tileLayer.getSize().y; y++) {
        // draw the tile
        const auto tile = tileLayer.getTiles()[x + y * tileLayer.getSize().x];
        if (tile.ID == 0) {
          continue;
        }

        // get the texture for this tile
        const auto &texture =
            this->textures[0]; // @TODO: support multiple tilesets

        // get the position of the tile
        const glm::vec2 position =
            glm::vec2(x * map.getTileSize().x, y * map.getTileSize().y);

        // get the xy index of the tile in the tileset
        const int tilesetColumns =
            texture->GetTextureRect().z / map.getTileSize().x;
        const int tileX = (tile.ID - 1) % tilesetColumns;
        const int tileY = (tile.ID - 1) / tilesetColumns;

        // create the src rect for the image from the tileset
        const glm::vec4 srcRect =
            glm::vec4(tileX * map.getTileSize().x, tileY * map.getTileSize().y,
                      map.getTileSize().x, map.getTileSize().y);

        // draw the tile
        spriteBatch->Draw(texture.get(), position, glm::vec2(1, 1), 0,
                          glm::vec4(1, 1, 1, 1), srcRect);

        if (DEBUG_COLLISIONS) {
          // get the tile type
          const auto &tileset = map.getTilesets()[0]; // @TODO: support multiple
                                                      // tilesets
          const auto &tilesetTile = tileset.getTile(tile.ID);

          if (tilesetTile->className == "SOLID") {
            // draw the collider as a red rect
            spriteBatch->DrawRect(glm::vec4(position.x, position.y,
                                            map.getTileSize().x,
                                            map.getTileSize().y),
                                  glm::vec4(1, 0, 0, 0.5f));
          }
        }
      }
    }
  }
}

void Tilemap::IsCollidingWith(SDL_Rect *other, SDL_Rect &found) {
  // get the bounding SDL Rect for the tilemap
  const SDL_Rect tilemapRect = {
      0, 0, static_cast<int>(map.getTileSize().x * map.getTileCount().x),
      static_cast<int>(map.getTileSize().y * map.getTileCount().y)};
  // if the other rect is not colliding with the tilemap, return
  if (!SDL_HasIntersection(other, &tilemapRect)) {
    return;
  }

  // get the possible tiles that could be colliding with the other rect
  const int startX = other->x / map.getTileSize().x;
  const int startY = other->y / map.getTileSize().y;
  const int endX = (other->x + other->w) / map.getTileSize().x;
  const int endY = (other->y + other->h) / map.getTileSize().y;

  // loop over the map's layers
  for (int i = 0; i < map.getLayers().size(); i++) {
    const auto &layer = map.getLayers()[i];
    // skip all layers that are not tile layers
    if (layer->getType() != tmx::Layer::Type::Tile) {
      continue;
    }
    // cast to a tile layer
    const auto tileLayer = layer->getLayerAs<tmx::TileLayer>();

    SDL_Rect compositeRect = {0, 0, 0, 0};

    // loop over all the possible tiles in the layer (x and y)
    for (int x = startX; x <= endX; x++) {
      for (int y = startY; y <= endY; y++) {
        // draw the tile
        const auto tile = tileLayer.getTiles()[x + y * tileLayer.getSize().x];
        if (tile.ID == 0) {
          continue;
        }

        // get the position of the tile
        const glm::vec2 position =
            glm::vec2(x * map.getTileSize().x, y * map.getTileSize().y);

        // get the xy index of the tile in the tileset
        const int tilesetColumns =
            textures[0]->GetTextureRect().z / map.getTileSize().x;
        const int tileX = (tile.ID - 1) % tilesetColumns;
        const int tileY = (tile.ID - 1) / tilesetColumns;

        // get the bounding SDL Rect for the tile
        const SDL_Rect tileRect = {static_cast<int>(position.x),
                                   static_cast<int>(position.y),
                                   static_cast<int>(map.getTileSize().x),
                                   static_cast<int>(map.getTileSize().y)};

        // if the other rect is not colliding with the tile, continue
        if (!SDL_HasIntersection(other, &tileRect)) {
          continue;
        }

        // get the tile type
        const auto &tileset = map.getTilesets()[0]; // @TODO: support multiple
                                                    // tilesets
        const auto &tilesetTile = tileset.getTile(tile.ID);

        if (!tilesetTile) {
          continue;
        }

        // if the tile class is "SOLID"
        if (tilesetTile->className == "SOLID") {
          // if composite rect is 0,0,0,0
          if (compositeRect.x == 0 && compositeRect.y == 0 &&
              compositeRect.w == 0 && compositeRect.h == 0) {
            // set composite rect to the tile rect
            compositeRect = tileRect;
          } else {
            // otherwise, combine the tile rect with the composite rect
            SDL_UnionRect(&compositeRect, &tileRect, &compositeRect);
          }
        }
      }
    }
    if (compositeRect.x != 0 || compositeRect.y != 0 || compositeRect.w != 0 ||
        compositeRect.h != 0) {
      found = compositeRect;
      // SDL_Log("other: %i, %i, %i, %i\n", other->x, other->y, other->w,
      //         other->h);
      // SDL_Log("found: %i, %i, %i, %i\n", found.x, found.y, found.w, found.h);
      return;
    }
  }
  return;
}
