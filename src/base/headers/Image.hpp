#pragma once

#include <string>
#include <atomic>
#include <Platform.hpp>
#include <Frame.hpp>
#include <Color.hpp>
#include <glm/vec2.hpp>
#include <memory>

namespace retro {

    class Game;
    class Image;

    typedef std::shared_ptr<Image> ImagePtr;

    /// Image that can be drawn
    /**
     * Loads an image that can be rendered anywhere. The formats supported are:
     *
     *  - JPEG baseline and progressive (12bpc/arithmetic not supported)
     *  - PNG 1/2/4/8/16 bit-per-channel
     *  - TGA (not all, but should work in general)
     *  - BMP non-1bpp and non-RLE
     *  - PSD (composited view only, no extra channels, 8/16 bit-per-channel)
     *  - GIF (always will be RGBA)
     *  - HDR (radiance rgbE format)
     *  - PIC (Softimage PIC)
     *  - PNM (PPM and PGM binary only)
     *
     * Can load images from files or from memory. Also, can use raw memory as
     * memory for textures, but take care using this method.
     *
     * The image can be drawn completely or a section of it. The methods
     * draw() can draw the image resized (or not) in the screen. The methods
     * drawSection() can draw a section of the image resized (or not) in the
     * screen. To draw an image, first you need to call either regenerate() or
     * generateAndDestroy(). Read carefully what does generateAndDestroy(),
     * is a good method, but the name is a bit scary.
     *
     * If you don't free the resources (through generateAndDestroy()), you can see
     * the pixels directly and even modify them. You have pixelAt() and modifyPixelAt()
     * that allows you to get and modify pixels safely. There's the rawPixelAt() that
     * returns a reference to the pixel as Color reference, it's a bit unsecure. In all
     * three methods, the position is not checked, so invalid positions could lead into
     * undefined behaviour and even to crash the game.
     *
     * @see https://github.com/nothings/stb/blob/master/stb_image.h stb_image.h
     */
    class Image {
        Game &game;
        void* data;
        void* surface;
        void* texture = nullptr;
        size_t width, height;
        bool linear = false, doNotFree = false;
        std::atomic_size_t& references;

    public:

        enum Channels {
            Undefined = 0,  ///< Only used int request
            RGB = 3,        ///< RGB (24bit integer)
            RGBA = 4        ///< RGBA (32bit integer)
        };

        /**
         * From the STB library:
         *  > (...) if you attempt to load an HDR file, it will be automatically remapped to
         *  > LDR, assuming gamma 2.2 and an arbitrary scale factor defaulting to 1;
         *  > both of these constants can be reconfigured through this interface:
         *
         * hdrSetProperties();
         *
         *  > (note, do not use _inverse_ constants; stbi_image will invert them
         *  > appropriately).
         *
         * Because HDR images will loose its powerful High Dynamic Range, this values
         * will set how to linearize the image to get a Low Dynamic Range version to
         * show.
         **/
        static void hdrSetProperties(float gamma = 2.2f, float scale = 1.0f);

        /// Loads an image from a file
        /**
         * Loads an image from a file in the resources folder, requesting (optionally)
         * a RGB or RGBA pixel format. If the file doesn't exists (or could not be read)
         * or the image has an unsupported format, will throw an exception. The
         * constructor won't generate the texture in the GPU, so before using it, call
         * regenerate().
         * @param path Path to the file
         * @param game Game reference
         * @param desired The number of channels desired to load
         * @see hdrSetProperties()
         */
        Image(const std::string &path, Game &game, Channels desired = Undefined);
        /// Loads an image from memory
        /**
         * Loads an image from memory through a pointer to the data, requesting (optionally)
         * a RGB or RGBA pixel format. If the image has an unsupported format, will
         * throw an exception. The constructor won't generate the texture in the GPU,
         * so before using it, call regenerate().
         * @param buffer Pointer to the image in memory
         * @param sizeInBytes Size of the memory chunk (in bytes)
         * @param game Game reference
         * @param desired The number of channels desired to load
         * @see hdrSetProperties()
         */
        Image(const void* buffer, size_t sizeInBytes, Game &game, Channels desired = Undefined);
        /// Uses a region of memory as raw image
        /**
         * The memory region must have an specific format. If you use Channels::RGB, the
         * pixels must be stored in bits like this: `BBBBBBBBGGGGGGGGRRRRRRRR` (24 bit
         * Little Endian). If, instead, you use Channels::RGBA, the pixels must be stored
         * like this: `AAAAAAAABBBBBBBBGGGGGGGGRRRRRRRR` (32 bit Little Endian). The
         * pointer to the data will not be freed by this class, it is your responsability
         * to free the data. But, the constructor won't copy the memory, so you must ensure
         * that the pointer is valid until the image is destroyed or until
         * generateAndDestroy() is called.
         * @param rawData Pointer to the raw image
         * @param size Size in pixels of the image (width x height)
         * @param channels Format of the image (RGB or RGBA)
         * @param game Reference to a Game instance
         */
        Image(uint32_t* rawData, const glm::uvec2 &size, Channels channels, Game &game);
        /// Copy constructor
        Image(const Image &image);
        /// Move constructor
        Image(Image &&image);
        ~Image();

        /// Loads an image using the first constructor of Image() and returns a pointer to it
        static ImagePtr loadImage(const std::string &path, Game &game, Channels desired = Undefined) {
            return ImagePtr(new Image(path, game, desired));
        }

        /// Gets the width of the image
        constexpr size_t getWidth() { return width; }
        /// Gets the height of the image
        constexpr size_t getHeight() { return height; }
        /// Gets the size (as glm::uvec2 vector) of the image
        constexpr glm::uvec2 getSize() { return { width, height }; }
        /// Gets the number of channels / pixel mode of the image
        constexpr Channels getPixelMode() { return channels; }
        /// Enables or disables linear sampling
        constexpr void enableLinearSampling(bool linear) { this->linear = linear; }
        /// Check if linear sampling is activated
        constexpr bool isLinearSamplingEnabled() { return linear; }

        /// Generate the GPU texture and frees any resources (cannot modify or get pixels)
        /**
         * This methods does the same as regenerate() (for the first time) but also frees
         * the raw pixels from the RAM. Doing that reduces the amount of used RAM, but you
         * cannot edit or get pixels using pixelAt(), modifyPixelAt() and rawPixelAt(),
         * also you cannot call regenerate(). It is recommended to use this method if you
         * won't use any of the upper methods. If you use any of the before methods, the
         * game will crash.
         *
         * You should **call** this method 0 or 1 times, no more.
         */
        void generateAndDestroy();

        /// Regenerate the GPU texture from the raw pixels
        /**
         * The first time that the image will be drawn, this method must be called.
         * Also, everytime that a pixel of the image is modified, should call this.
         * Regenerates the texture from the raw pixels directly into the GPU.
         */
        void regenerate();

        /// Draws the entire image into the frame (resizing it)
        /**
         * The image will be drawn in the position and size specified by `frame`.
         * If the size is lower or higher than the image, a resize of it will ocurr
         * in the rendering. Will apply `nearest` resampling (if linear is disabled),
         * where the pixels are selected without any interpolation, making the image
         * to be a bit pixelated when resizing, perfect for pixel art. If linear is
         * enabled, the pixels will look better when resizing, but will loose the
         * _pixel art_ mode.
         * @param frame What size and where to draw the image
         */
        void draw(const Frame &frame);

        /// Draws the entire image into the frame (without resizing)
        /**
         * The image will be drawn in the position and size specified by `pos`.
         * @param pos Where to draw the image
         */
        void draw(const glm::vec2 &pos);

        /// Draws the a section of the image into the frame (resizing it)
        /**
         * The draw will grab a region defined in `section` of the original image
         * and draw it in the position and size specified by `frame`.
         * If the size is lower or higher than the section, a resize of it will ocurr
         * in the rendering. Will apply `nearest` resampling (if linear is disabled),
         * where the pixels are selected without any interpolation, making the section
         * to be a bit pixelated when resizing, perfect for pixel art. If linear is
         * enabled, the pixels will look better when resizing, but will loose the
         * _pixel art_ mode.
         * @param section The section of the image to draw
         * @param whereToDraw What size and where to draw the section
         */
        void drawSection(const Frame &section, const Frame &whereToDraw);

        /// Draws the a section of the image into the frame
        /**
         * The draw will grab a region defined in `section` of the original image
         * and draw it in the position specified by `frame`.
         * @param section The section of the image to draw
         * @param whereToDraw Where to draw the section
         */
        void drawSection(const Frame &section, const glm::vec2 &whereToDraw);

        /// Gets a pixel from that position
        /**
         * This operation is unsafe, meaning that if the position is invalid, the
         * behaviour is undefined.
         */
        Color pixelAt(size_t x, size_t y) const;
        /// Modifies a pixel from that position with a new color
        /**
         * This operation is unsafe, meaning that if the position is invalid, the
         * behaviour is undefined.
         */
        void modifyPixelAt(size_t x, size_t y, Color c);
        /// Gets a reference to the raw pixel from that position
        /**
         * This operation is unsafe, meaning that if the position is invalid, the
         * behaviour is undefined. The value returned has the following structure:
         *
         *  - **RGBA**: `AAAAAAAABBBBBBBBGGGGGGGGRRRRRRRR` (32 bit)
         *  - **RGB**: `rrrrrrrrBBBBBBBBGGGGGGGGRRRRRRRR` (32 bit)
         *
         * **Observe** that in `RGB`, we cannot represent a 24 bit integer, so the
         * previous red pixel will be inside the raw value. Try to not modify the
         * alpha value.
         */
        Color& rawPixelAt(size_t x, size_t y);

        /// Copy-assign operator
        Image& operator=(const Image&);
        /// Move-assign operator
        Image& operator=(Image&&);

    private:
        Channels channels;
    };

}
