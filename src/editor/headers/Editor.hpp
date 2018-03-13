#pragma once

#include <vector>
#include <Game.hpp>
#include <Sprites.hpp>
#include <Map.hpp>

namespace retro {
    
    /// Editor UI app. Come in and see how to use it.
    /**
     * By default enters in Sprite mode. The editor has different modes
     * depending on what is editing. You can switch between them using:
     *
     *  - `Ctrl+1`: Sprite editor
     *  - `Ctrl+2`: Map editor
     *
     * In macOS, use `⌘` instead of `Ctrl`. In Linux you can use `Meta` if you want.
     *
     * When editing a Sprites file:
     *  - When **right click** on a pixel of a sprite, selects this colour
     *  - When **left click** on a pixel when drawing, puts the selected colour
     *  - When **left click** on a pixel with the cursor, selects a region
     *  - When a region is selected, `Ctrl+C`/`⌘+C` copies the region
     *  - When `Ctrl+V`/`⌘+V` is pressed, pastes the copied region at (0,0) of the
     * selected sprite. If the mouse is over a pixel, pastes it starting at this
     * position. Paste something that will be larger than the sprite, simply cuts
     * the extra pixels and doesn't draw in the near sprites.
     *  - When a region is selected, pressing `Supr`/`fn+←` deletes the contents.
     *  - To erase a pixel, use the first colour in the palette, is fully transparent,
     * in fact, by default all sprites are transparent, but the background of the canvas
     * is black.
     *  - You can increase the number of sprites in a file, but not decrease it.
     *  - **Remember** that a Map can only address up to 255 sprites and 0
     * is reserved for transparent sprite.
     *  - When `Ctrl+S`/`⌘+S` is pressed, saves the changes to the file.
     *  - When `Ctrl+R`/`⌘+R` is pressed, restores the changes from the file, discarding
     * any changes done.
     *
     * When editing a Map file:
     *  - Scrolling in the canvas moves the map in Y axis. If you use a Mac with a
     * trackpad or with the Magic Mouse, you can also move with the X axis easily.
     *  - Scrolling with Shift in the canvas pressed moves the map in X axis. If
     * you use a Mac with a trackpad or with the Magic Mouse, the axis are inverted.
     *  - When **right click** on a cell, selects that sprite (except if there's no sprite there)
     *  - When **left click** on a pixel when drawing, puts the selected colour
     *  - When **left click** on a pixel with the cursor, selects a region
     *  - When a region is selected, `Ctrl+C`/`⌘+C` copies the region
     *  - When `Ctrl+V`/`⌘+V` is pressed, pastes the copied region at the mouse
     * position. If the mouse is not inside the canvas, does nothing.
     *  - When a region is selected, pressing `Supr`/`fn+←` deletes the contents.
     *  - When `Ctrl+S`/`⌘+S` is pressed, saves the changes to the file.
     *  - When `Ctrl+R`/`⌘+R` is pressed, restores the changes from the file, discarding
     * any changes done.
     *
     * Have fun :)
     **/
    class Editor: public Game {
        
        Optional<Map> mapFile;
        Optional<Sprites> spriteFile;
        
        enum { MAP, SPRITE } mode = SPRITE;
        const Level* mapLevel = nullptr;
        const Level* spriteLevel = nullptr;

    protected:
        virtual void setup();
        virtual void cleanup();
        
    public:
        Editor(const Builder &builder);
        void setPalette(const char*);
        void setFont(const char*);
        
        void changeToNewMapFile();
        void changeToMapFileSelector();
        void changeToSpriteEditor(const std::string &map);
        void changeToNewSpriteFile();
        void changeToSpriteFileSelector();
        void changeToMapEditor(const std::string &);
        
        void checkChangeModeInput(GameActions&, int);
        void changeToMapMode();
        void changeToSpriteMode();

    };

}
