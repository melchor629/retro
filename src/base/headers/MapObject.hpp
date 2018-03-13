#pragma once

#include <Map.hpp>
#include <Object.hpp>
#include <vector>

namespace retro {

    /// A map, but as an Object.
    class MapObject: public Map, public Object {

        std::vector<uint8_t> invalidSprites = { 0 };

    public:

        /**
         * Loads a Map with functionality of an Object. Instead of the name, you must set the
         * map. The name will be the file name.
         **/
        MapObject(Game &game, Level &level, const glm::vec2 &pos, const std::string &path): Map(path, game), Object(game, level, pos, path.substr(path.rfind('/') + 1, path.rfind('.'))) {}

        virtual void setup() override {
            regenerateTextures();
            frame.size = Map::getSize() * 8u;
        }

        virtual void update(float, GameActions &) override {}

        virtual void draw(GameActions&) override {
            Map::draw(frame);
        }

        /// Sets the position of the map
        inline void setPosition(const glm::ivec2 &pos) {
            frame.pos = pos;
        }

        /// Updates the invalid sprite list with a new list of values. Invalid sprites are for collision detection.
        inline void updateInvalidSprites(std::initializer_list<uint8_t> list) {
            invalidSprites.clear();
            invalidSprites.insert(invalidSprites.begin(), list);
        }

        /// Adds a new invalid sprite to the list. Invalid sprites are for collision detection.
        inline void addInvalidSprites(uint8_t s) {
            invalidSprites.push_back(s);
        }

        /// Checks if this position is a valid position or its in a cell with an invalid sprite. If the position is outside the bounds of the map, it will return `true`.
        inline bool validPosition(const glm::ivec2 &pos) {
            glm::ivec2 mapPos = (pos - glm::ivec2(frame.pos)) / 8;
            if(mapPos.x >= 0 && mapPos.y >= 0 && mapPos.x < int(getSize().x) && mapPos.y < int(getSize().y)) {
                return std::find(invalidSprites.begin(), invalidSprites.end(), at(mapPos.x, mapPos.y)) == invalidSprites.end();
            } else {
                return true;
            }
        }

    };

}
