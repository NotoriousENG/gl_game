#include "tilemap.hpp"
#include "SDL2/SDL_log.h"
#include "glad/glad.h";

Tilemap::Tilemap(const char *path) {
  this->map.load(path);

  // load the textures
  const auto &tilesets = map.getTilesets();
  for (const auto &ts : tilesets) {
    this->textures.push_back(
        std::make_shared<Texture>(ts.getImagePath().c_str()));
  }
}

Tilemap::~Tilemap() {
  for (auto &ss : this->subsets) {
    if (ss.vbo) {
      glDeleteBuffers(1, &ss.vbo);
    }
    if (ss.lookup) {
      glDeleteTextures(1, &ss.lookup);
    }
  }
}

// @TODO: use a pixel buffer and do this on the GPU instead of drawing a bunch of quads
void Tilemap::Draw(SpriteBatch* spriteBatch) {
  // loop over the map's layers
  for (int i = 0; i < map.getLayers().size(); i++) {
    auto &layer = map.getLayers()[i];
    // skip all layers that are not tile layers
    if (layer->getType() != tmx::Layer::Type::Tile) {
      continue;
    }

    // cast to a tile layer
    auto tileLayer = layer->getLayerAs<tmx::TileLayer>();

    // loop over all the tiles in the layer (x and y)
    for (int x = 0; x < tileLayer.getSize().x; x++) {
      for (int y = 0; y < tileLayer.getSize().y; y++) {
          // draw the tile
          auto tile = tileLayer.getTiles()[x + y * tileLayer.getSize().x];
          if (tile.ID == 0) {
			continue;
		  }

          // get the subset for this tile
		  auto &subset = this->subsets[tile.ID - 1];
		  // get the texture for this tile
		  auto &texture = this->textures[0];
		  // get the position of the tile
		  glm::vec2 position = glm::vec2(x * map.getTileSize().x,
              										 y * map.getTileSize().y);

          // get the xy index of the tile in the tileset
          int tileX = (tile.ID - 1) % (texture->GetTextureRect().z / map.getTileSize().x);
          int tileY = (tile.ID + 1) / (texture->GetTextureRect().w /
                                               map.getTileSize().y);

          // create the src rect for the image from the tileset
          glm::vec4 srcRect = glm::vec4(tileX * map.getTileSize().x,
              										tileY * map.getTileSize().y,
              										map.getTileSize().x,
              										map.getTileSize().y);

		  // draw the tile
          spriteBatch->Draw(texture.get(), position, glm::vec2(1, 1), 0, glm::vec4(1, 1, 1, 1), srcRect);
      }
    }
  }
}
