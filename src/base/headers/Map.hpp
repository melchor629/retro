#pragma once

#include <stdint.h>
#include <atomic>
#include <string>
#include <Frame.hpp>
#include <Sprites.hpp>

struct SDL_Surface;
struct SDL_Texture;

namespace retro {

    class Game;
    class Palette;

    /// A map file, but as a C++ object.
    /**
     * A map is just numbers and a reference to a `.spr` file (a Sprites file).
     * Stores only cells of 8x8 canvas pixels, and every cell references a sprite
     * of that file. Can only reference up to 256 sprites.
     **/
    class Map {

        Game &game;
        uint8_t* &data;
        glm::uvec2 size;
        std::string path;
        Sprites* sprites;
        uint32_t* pixels;
        SDL_Surface* surface;
        SDL_Texture* texture;
        std::atomic_size_t& references;

    public:

        /// Loads a `.map` from the game path folder
        Map(const std::string &path, Game &g);
        /// Copies an instance of Map
        Map(const Map &map);
        /// Moves an instance of Map
        Map(Map &&map);
        /// Gets rid of an instance of Map
        ~Map();

        /// Returns the size of the map (not in pixels, in map cells).
        inline const glm::uvec2 getSize() const { return size; }
        /// Returns a reference of the sprite value (from the memory) located in a map cell. 0 is for transparent sprite and the rest is the `sprite index+1`.
        uint8_t& at(size_t x, size_t y);
        /// Returns the sprite value located in a map cell.
        constexpr const uint8_t& at(size_t x, size_t y) const { return data[y * size.x + x]; }
        /// Returns the sprite value located in a map cell.
        constexpr const uint8_t& at(const glm::ivec2 &pos) const { return at(pos.x, pos.y); }
        void resize(const glm::uvec2 &size);
        /// Regenerates the textures to match the changes done in the map.
        void regenerateTextures();
        /// Saves the changes done in the map.
        void save();
        /// Reloads the map from the `.map` file.
        void reload();
        /// Draws the map inside a Frame (using canvas pixels, not map cells).
        void draw(const Frame &frame);
        /// Get the Sprites object used in this map.
        const Sprites* getSprites() const;

        static Map createMap(const std::string &path, Game &g, const Sprites &sprites, const glm::uvec2 &initialSize = { 128, 32 });

    };

}
