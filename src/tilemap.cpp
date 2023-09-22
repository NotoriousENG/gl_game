#include "tilemap.hpp"
#include "SDL2/SDL_log.h"
#include "glad/glad.h"
#include "tmxlite/Map.hpp"
#include "tmxlite/TileLayer.hpp"

Tilemap::Tilemap(const char *path) {
  tmx::Map map;
  map.load(path);

  // load the textures
  const auto &tilesets = map.getTilesets();
  for (const auto &ts : tilesets) {
    this->textures.push_back(
        std::make_shared<Texture>(ts.getImagePath().c_str()));
  }

  // create a drawable object for each tile layer
  const auto &layers = map.getLayers();
  for (auto i = 0u; i < layers.size(); ++i) {
    if (layers[i]->getType() == tmx::Layer::Type::Tile) {
      const auto layer = dynamic_cast<const tmx::TileLayer *>(layers[i].get());
      tmx::FloatRect bounds = map.getBounds();
      float verts[] = {bounds.left,
                       bounds.top,
                       0.f,
                       0.f,
                       0.f,
                       bounds.left + bounds.width,
                       bounds.top,
                       0.f,
                       1.f,
                       0.f,
                       bounds.left,
                       bounds.top + bounds.height,
                       0.f,
                       0.f,
                       1.f,
                       bounds.left + bounds.width,
                       bounds.top + bounds.height,
                       0.f,
                       1.f,
                       1.f};

      const auto &mapSize = map.getTileCount();
      const auto tilesets = map.getTilesets();
      for (auto j = 0u; j < tilesets.size(); ++j) {
        const auto &ts = tilesets[j];
        const auto &tileIDs = layer->getTiles();
        std::vector<std::uint16_t> pixelData;
        bool tsUsed = false;

        for (auto y = 0u; y < mapSize.y; ++y) {
          for (auto x = 0u; x < mapSize.x; ++x) {
            auto idx = y * mapSize.x + x;
            if (idx < tileIDs.size() && tileIDs[idx].ID >= ts.getFirstGID() &&
                tileIDs[idx].ID < (ts.getFirstGID() + ts.getTileCount())) {
              pixelData.push_back(static_cast<std::uint16_t>(tileIDs[idx].ID -
                                                             ts.getFirstGID()) +
                                  1); // red channel - making sure to index
                                      // relative to the tileset
              pixelData.push_back(static_cast<std::uint16_t>(
                  tileIDs[idx].flipFlags)); // green channel - tile flips are
                                            // performed on the shader
              tsUsed = true;
            } else {
              // pad with empty space
              pixelData.push_back(0);
              pixelData.push_back(0);
            }
          }
        }

        if (tsUsed) {
          this->subsets.emplace_back();
          this->subsets.back().texture = this->textures[j]->GetGLTexture();

          glGenBuffers(1, &this->subsets.back().vbo);
          glBindBuffer(GL_ARRAY_BUFFER, this->subsets.back().vbo);
          glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

          glGenTextures(1, &this->subsets.back().lookup);
          glBindTexture(GL_TEXTURE_2D, this->subsets.back().lookup);
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16UI, mapSize.x, mapSize.y, 0,
                       GL_RG_INTEGER, GL_UNSIGNED_SHORT,
                       (void *)pixelData.data());

          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
      }
    }
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
