#pragma once

#include <string>
#include <atomic>
#include <stdexcept>
#include <Frame.hpp>

struct SDL_Surface;
struct SDL_Texture;
struct SDL_Renderer;

namespace retro {

    class Palette;
    class Game;
    class Map;
    class Level;

    /// A Sprite, of any size.
    struct Sprite {
        const size_t index; ///< Index of the sprite. Starts at 0.
        const size_t width; ///< Width of the sprite (can be 8, 16, 32 or 64)
        const size_t height; ///< height of the sprite (can be 8, 16, 32 or 64)
        const class Sprites &origin; ///< Sprites object where this Sprite comes from.

        inline uint8_t& operator[](size_t i) const;
        inline uint8_t& at(size_t x, size_t y) const;
        /// Changes the size of the sprite at compile-time.
        template<size_t Size = 8>
        inline const Sprite size() const {
            static_assert(Size==8||Size==16||Size==32||Size==64, "Size only can be 8, 16, 32 or 64");
            return { index, Size, Size, origin };
        }
        /// Changes the size of the sprite at runtime.
        inline const Sprite size(size_t t) const {
            if(t == 8)  return size<8>();
            if(t == 16) return size<16>();
            if(t == 32) return size<32>();
            if(t == 64) return size<64>();
            throw std::runtime_error("Size only can be 8, 16, 32 or 64");
        }
        /// Draws the sprite, where it's size can be modified when drawn.
        /**
         * The size of the frame is measured in multiple of 8 or in multiples
         * of 1 normal sprite. That means that if the size is { 3, 3 }, it will
         * draw an 24x24 pixel sprite (1 normal sprite is 8x8).
         * @param frame Frame where to draw the sprite
         **/
        void draw(const Frame &frame) const;
        /// Draws the sprite at this position, without modifing its size.
        void draw(const glm::vec2 &pos) const;
        /// Draws the sprite at this position, rotated and (optionally) flipped vertically (1),
        /// horizontally (2) or both (3)
        void draw(const glm::vec2 &pos, double rotation, int flip = 0) const;
        /// Draws the sprite at this position, rotated using a different center, and
        /// (optionally) flipped vertically (1), horizontally (2) or both (3)
        void draw(const glm::vec2 &pos, double rotation, const glm::ivec2 &center, int flip = 0) const;
        void draw_thicc(const Frame &frame) const;
        /// Gets the Frame of the Sprite.
        Frame frame() const;
    };

    /// A sprites file, but as a C++ object.
    /**
     * A Sprites file (`.spr`) is a file that contains 16 sprites in a row, and multiple rows.
     * A sprite is a 8x8 pixels where every pixel refers to an index of a colour from
     * a Palette. Can only reference 256 colours.
     **/
    class Sprites {

        Game &game;
        uint8_t* data;
        uint64_t sprites;
        std::string path;
        uint32_t* pixels;
        SDL_Surface* surface;
        SDL_Texture* texture;
        std::atomic_size_t& references;
        friend struct Sprite;
        friend Map;

        SDL_Renderer* renderer() const;
        Level* currentLevel() const;
        Frame frameSprite(const Sprite* spr, float &percx, float &percy) const;

    public:

        /// Loads a `.spr` from the game path, or creates a new one with 64 empty sprites.
        Sprites(const std::string &path, Game &game);
        /// Copies an instance of Sprites
        Sprites(const Sprites &other);
        /// Moves an instance of Sprites
        Sprites(Sprites &&old);
        /// Creates an instance of Sprites unitialized. Use load() to initialize it.
        Sprites(Game &game);
        ~Sprites();

        /// Obtains a Sprite by index.
        const Sprite operator[](size_t n);
        /// Obtains a Sprite by index.
        constexpr const Sprite operator[](size_t n) const { return Sprite { n, 8, 8, *this }; }
        /// Obtains a Sprite by index.
        constexpr const Sprite at(size_t n) const { return (*this)[n]; }
        /// Adds 16 more sprites at the end of the file.
        void addSpritesRow();
        /// Saves everything to the file.
        void save() const;
        /// Reloads the sprites from the file, discarding any changes.
        void reload();
        /// Regenerates the textures that will be used to draw the sprites.
        void regenerateTextures();
        /// Returns the number of sprites. Will be always multiple of 16.
        size_t size() const;
        /// Loads a `.spr` from the game path, or creates a new one with 64 empty sprites.
        void load(const std::string &path) throw();

    };

    inline uint8_t& Sprite::operator[](size_t i) const {
        size_t x = (index % 16) * 8 + i % width;
        size_t y = (index / 16) * 8 + i / width;
        return origin.data[x + y * 8 * 16];
    }

    inline uint8_t& Sprite::at(size_t x, size_t y) const {
        return origin.data[((index % 16) * 8 + x) + ((index / 16) * 8 + y) * 8 * 16];
    }

}
