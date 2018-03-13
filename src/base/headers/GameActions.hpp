#pragma once

#ifndef _SDL_IMPORTED_
#define _SDL_IMPORTED_
#define SDL_BUTTON_LEFT     1 ///< Mouse left button left (from SDL)
#define SDL_BUTTON_MIDDLE   2 ///< Mouse middle button (from SDL)
#define SDL_BUTTON_RIGHT    3 ///< Mouse right button (from SDL)
#define SDL_BUTTON_X1       4 ///< Mouse extra 1 button (from SDL)
#define SDL_BUTTON_X2       5 ///< Mouse extra 2 button (from SDL)

#if !defined(_WIN32) and !defined(__ANDROID__) and !defined(__IOS__)
#include <SDL2/SDL_keycode.h>
#else
#include <SDL_keycode.h>
#endif
#endif

#include <Color.hpp>

namespace retro {

    class Game;
    class Level;
    struct Frame;

    /// Some actions that you can do inside a Level.
    class GameActions {

        friend Game;
        friend class UIObject;
        friend struct Sprite;
        friend class Map;
        friend class Image;
        Game &g;
        Level &l;
        bool doubleIt = true;

    public:

        GameActions(Game &g, Level &l);

        /// Clear all the screen using a RGB color.
        void clear(const Color &color);
        /// Clear all the screen using a color from the palette.
        void clear(size_t color);
        /// Sets a RGBA color to be used in the next draw calls.
        void setColor(const Color &color);
        /// Sets a color from the palette to be used in the next draw calls.
        void setColor(size_t color);
        /// Draws a rectangle using the fixed colour.
        void drawRectangle(const Frame &frame);
        /// Draws a rectangle using a color from the palette.
        void drawRectangle(const Frame &frame, size_t color);
        /// Draws a rectangle using a RGBA colour.
        void drawRectangle(const Frame &frame, const Color &color);
        /// Draws a filled rectangle using the fixed colour.
        void fillRectangle(const Frame &frame);
        /// Draws a filled rectangle using a color from the palette.
        void fillRectangle(const Frame &frame, size_t color);
        /// Draws a filled rectangle using a RGBA colour.
        void fillRectangle(const Frame &frame, const Color &color);
        /// Draws a line from an initial position to a final position using the fixed colour.
        void drawLine(const glm::vec2 &ipos, const glm::vec2 &epos);
        /// Draws a line from an initial position to a final position using a colour from the palette.
        void drawLine(const glm::vec2 &ipos, const glm::vec2 &epos, size_t color);
        /// Draws a line from an initial position to a final position using a RGBA colour.
        void drawLine(const glm::vec2 &ipos, const glm::vec2 &epos, const Color &color);
        /// Shows a text in a position using the fixed colour.
        void print(const std::string &str, const glm::vec2 &pos);
        /// Shows a text in a position using a colour from the palette.
        void print(const std::string &str, const glm::vec2 &pos, size_t color);
        /// Shows a text in a position using a RGBA colour.
        void print(const std::string &str, const glm::vec2 &pos, const Color &color);
        /// Calculates the size of the text shown in the screen.
        glm::ivec2 sizeOfText(const std::string &str);
        /// Puts the fixed colour on a pixel.
        void putColor(const glm::vec2 &pos);
        /// Puts a colour from the palette on a pixel.
        void putColor(const glm::vec2 &pos, size_t color);
        /// Puts a RGBA colour on a pixel.
        void putColor(const glm::vec2 &pos, const Color &color);
        /// Draws a empty circle with center `pos` and a radius, using the fixed colour.
        void drawCircle(const glm::vec2 &pos, float radius);
        /// Draws a circle with center `pos` and a radius, using a colour from the palette.
        void drawCircle(const glm::vec2 &pos, float radius, size_t color);
        /// Draws a circle with center `pos` and a radius, using a RGBA colour.
        void drawCircle(const glm::vec2 &pos, float radius, const Color &color);
        /// Draws a filled circle with center `pos` and a radius, using the fixed colour.
        void fillCircle(const glm::vec2 &pos, float radius);
        /// Draws a filled circle with center `pos` and a radius, using a colour from the palette.
        void fillCircle(const glm::vec2 &pos, float radius, size_t color);
        /// Draws a filled circle with center `pos` and a radius, using a RGBA colour.
        void fillCircle(const glm::vec2 &pos, float radius, const Color &color);
        /// Enables a rect-clip in a region defined by the Frame.
        /**
         * When a rect-clip is set, everything that will be drawn outside it, will be clipped,
         * so won't be drawn. Useful to hide some stuff when doing some compositioning of layers.
         * You must explicity disable the cliping with disableClipInRectangle().
         * @param rect The Frame of the rect-clip
         **/
        void enableClipInRectangle(const Frame &rect);
        /// Disables the rect-clip
        void disableClipInReactangle();

        void drTHICC(const Frame &frame, const Color &color);
        void dlTHICC(const glm::vec2 &ipos, const glm::vec2 &epos, const Color &color);
        const glm::ivec2 mpTHICC();
        const glm::ivec2 mpProfiteroles();

        /// Changes the position of the camera
        void camera(const glm::vec2 &pos);
        /// Gets the position of the camera
        glm::vec2 camera();

        /// Checks whether a key is pressed or not. See https://wiki.libsdl.org/SDL_Scancode for the valid values.
        bool isKeyPressed(int key); //Use SDL_SCANCODE_*
        /// Checks whether a modifier key is pressed or not. See https://wiki.libsdl.org/SDL_Keymod for the valid values
        bool isModKeyPressed(int mod); //Use KMOD_*
        /// Checks whether a mouse button is pressed or not. Valid values start by `SDL_BUTTON_`.
        bool isMousePressed(int key); //Use SDL_BUTTON_*
        /// Return the buttons pressed in the mouse. Valid values start by `SDL_BUTTON_`.
        int getMousePressedKey();
        /// Gets the mouse position
        glm::ivec2 getMousePosition();

        std::vector<glm::vec2> getTouchPositions();

        void captureMouse(bool capture);
        void startInputText(const Frame &editTextRegion);
        void endInputText();

        /// Gets the canvas size, in game size, not in the real.
        const glm::uvec2 canvasSize();

    };

}
