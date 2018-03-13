#pragma once
#define _retro_game_hpp_

#include <string>
#include <type_traits>
#include <map>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <Frame.hpp>
#include <Palette.hpp>
#include <Logger.hpp>
#include <Platform.hpp>

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

typedef struct _TTF_Font TTF_Font;
typedef struct SDL_Window SDL_Window;
struct SDL_Renderer;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct Mix_Chunk Mix_Chunk;
typedef struct _Mix_Music Mix_Music;
struct DisplayMode {
    uint32_t format;
    uint32_t width;
    uint32_t height;
    uint32_t refreshRate;
    void *driverData;
};

namespace retro {

    class GameActions;
    class Level;
    class Sprites;
    class Map;
    class UIObject;
    class Timer;
    class Image;

    /// The game. Everything lies on it.
    /**
     * Represents a game by itself, stores all levels and allows you
     * to manipulate the window settings. Is where you initialize the
     * game and starts to run.
     *
     * To create a game you must extend Game an implement the following methods:
     *
     *  - Game() (yep that's the constructor, but you must create it)
     *  - setup()
     *  - cleanup()
     *
     * To make an instance of it, you must use the Builder.
     *
     * Check also getWindow().
     **/
    class Game {
    public:
        enum class CanvasMode: uint16_t {
            FreeMode = 0, ///< The width and height of the canvas will be the size of the window divided by 10
            UltraLowSize = 64, ///< Equivalent to 64 x 36 (height is adaptable)
            LowSize = 85, ///< Equivalent to 85 x 48 (height is adaptable)
            NormalSize = 128, ///< Equivalent to 128 x 72 (height is adaptable)
            HighSize = 192, ///< Equivalent to 192 x 108 (height is adaptable)
            UltraHighSize = 205 ///< Equivalent to 205 x 115 (height is adaptable)
        };

        /// Game Builder :)
        class Builder {
            std::string name = "Game Demo";
            std::string gamePath;
            Frame frame;
            bool visible;
            bool resizable;
            Optional<DisplayMode> dp;
            int dp_flag = 0;
            int sampleRate = 0;
            int channels = 0;
            int audioChunkSize = 0;
            CanvasMode canvasMode = CanvasMode::NormalSize;
            friend Game;

        public:
            Builder();
            /// Sets the position and the size of the window.
            Builder& setFrame(const Frame &frame);
            /// Sets the position of the window (by default is platform dependent).
            Builder& setPosition(const glm::uvec2 &pos);
            /// Sets the size of the window.
            Builder& setSize(uint32_t width, uint32_t height);
            /// Sets the name of the Game (and the name of the Window).
            Builder& setName(const std::string &name);
            /// Sets fullscreen mode using Fullscreen or Windowed.
            Builder& setFullscreen(DisplayMode dp, bool fs_desktop = false);
            /// Sets if the window is visible at the beginning or not
            Builder& setVisible(bool visible);
            /// Sets whether the window will be resizable or not
            Builder& setResizable(bool resizable);
            /// Sets the game path.
            /**
             * By default, this path is the current working directory (the one where
             * the executable is called). In `DEBUG` mode, the path points to the `res`
             * folder in the root of the repository. Once changed, you can't revert to
             * the default value easily.
             * @param gamePath new game path
             **/
            Builder& setGamePath(std::string gamePath);
            /// Enables audio for the game, disabled by default. If you know what you are doing,
            /// you can change the default values :) Sorry, but you cannot provide audio in
            /// 24 bit integer, or 32 bit float :(, only 16 bit signed integers. `audioChunkSize`
            /// is in bytes.
            Builder& enableAudio(int sampleRate = 44100, int channels = 2, int audioChunkSize = 2048);
            /// Changes the CanvasMode to the one selected
            Builder& changeCanvasMode(CanvasMode mode);
            /// Creates an instance of the Game. You must `delete` the pointer at the end.
            template<class GameClass> GameClass* build();

            /// Allows you to query a DisplayMode for one monitor
            Optional<DisplayMode> getDisplayMode(int monitor, int mode);
            /// Gets the current display mode
            Optional<DisplayMode> getCurrentDisplayMode();
        };

        /// Manages the window
        class Window {
            friend Game;
            SDL_Window* window;
            const float &scaleFactor;
            Logger &log;
            
            Window(SDL_Window* window, Logger &log, const float &scaleFactor);
            
        public:

            /// Returns true if the window is resizable
            bool isResizable();
            /// Changes the resizable state of the window
            void setResizable(bool);
            /// Changes the app icon to the one inside in `pixels`
            void setIcon(void* pixels, const glm::uvec2 &size, uint8_t bitDepth, uint8_t channels);
            /// Gets the size of the window
            glm::uvec2 getSize();
            /// Changes the size of the window
            void setSize(const glm::uvec2 & size);
            /// Gets the opacity of the window (on some platforms will always be 1)
            float getOpacity();
            /// Changes the opacity of the window (on some platforms won't have any effect)
            void setOpacity(float op);
            /// Return `true` if the window has borders
            bool isBordered();
            /// Changes between a window with borders and one without
            void setBordered(bool);
            /// Gets the position of the window (in general, the top-left corner)
            glm::ivec2 getPosition();
            /// Sets the position of the window (in general, the top-left corner)
            void setPosition(const glm::ivec2 &pos);
            /// Gets the minimum size of the window
            glm::uvec2 getMinimumSize();
            /// Changes the minimum size of the window
            void setMinimumSize(const glm::uvec2 &size);
            /// Gets the maximum size of the window
            glm::uvec2 getMaximumSize();
            /// Changes the maximum size of the window
            void setMaximumSize(const glm::uvec2 &size);
            /// Returns `true` if the window is in (real) fullscreen
            bool isFullscreenMode();
            /// Returns `true` if the window is in windowed fullscreen
            bool isWindowedFullscreenMode();
            /// Returns `true` if the window is windowed
            bool isWindowedMode();
            /// Changes to fullscreen mode and tells you if it has been changed
            bool changeToFullscreenMode();
            /// Changes to windowed fullscreen mode and tells you if it has been changed
            bool changeToWindowedFullscreenMode();
            /// Changes to windowed mode and tells you if it has been changed
            bool changeToWindowedMode();
            /// Gets the a DisplayMode for the given monitor
            Optional<DisplayMode> getDisplayMode(int monitor, int mode);
            /// Gets the current DisplayMode
            Optional<DisplayMode> getCurrentDisplayMode();
            /// Sets the current DisplayMode to the one given
            bool setCurrentDisplayMode(const DisplayMode &dp);
            /// Gets the window title
            const char* getTitle();
            /// Sets the window title
            void setTitle(const std::string &title);
            /// Shows the window
            void show();
            /// Hides the window
            void hide();
            /// Tells you if the window is hidden or not
            bool isHidden();
            /// Gets the scale factor used in the UI
            float getScaleFactor();
            
        };

        /// Manages the audio, if enabled.
        class Audio {

            friend class Game;
            Logger &log;
            bool enabled;
            const std::string &gamePath;
            std::map<std::string, Mix_Chunk*> samples;
            std::map<std::string, Mix_Music*> musics;

            Audio(Logger &log, bool enabled, const std::string &gamePath);

            Mix_Chunk* findChunk(const std::string &);
            Mix_Music* findMusic(const std::string &);

        public:

            ~Audio();

            /// Get last error
            const char* getLastError();

            /// Loads a sample from a file and assigns the name of the file (without extension)
            /// as name of the sample. Supported formats are WAV, AIFF, OGG and VOC. If a file
            /// the same name is already loaded, will overwrite the previous one.
            void loadSample(const std::string &path);
            /// Loads a sample from a file and assigns a name for the sample.
            /// Supported formats are WAV, AIFF, OGG and VOC. If a sample with the same name
            /// exists, will overwrite the previous one.
            void loadSample(const std::string &path, const std::string &name);
            /// Loads a sample from RAW audio memory. The audio must be in the same format of
            /// the output (by default, 44100Hz, 16bps and 2 channels). Makes a copy of the data.
            /// If a sample with the same name already exists, will be overwritten.
            void loadSample(void* memory, const std::string &name);

            /// Loads a music from a file and assigns the name of the file (without extension)
            /// as name of the music. Supports WAV, MOD, MIDI, OGG, MP3 and FLAC. If a file
            /// the same name is already loaded, will overwrite the previous one.
            void loadMusic(const std::string &path);
            /// Loads a music from a file and assigns a name for it. Supports
            ///  WAV, MOD, MIDI, OGG, MP3 and FLAC. If a sample with the same name
            /// exists, will overwrite the previous one.
            void loadMusic(const std::string &path, const std::string &name);

            /// By default, there are 8, probably enough for everything, but you can use up to
            /// 255 sample mixers. In one channel, you can only play one sample. To make a mix
            /// of it, you must play different samples in different channels.
            void changeNumberOfChannels(uint8_t num);
            /// Sets the volume for a channel. The minimum volume is 0 and the maximum is 128.
            void setChannelVolume(uint8_t ch, uint8_t volume);
            /// Gets the channel volume.
            uint8_t getChannelVolume(uint8_t ch);
            /// Plays a sample in the first free channel, `loops + 1` times. If `loops` is -1,
            /// it will play continiusly after a stopSample() is called or something that makes
            /// it stop. Returns the channel where the sample is playing or -1 if error.
            int playSample(const std::string &sample, int loops = 0);
            /// Plays a sample in `ch` channel, `loops + 1` times. If `loops` is -1,
            /// it will play continiusly after a stopSample() is called or something that makes
            /// it stop.
            bool playSampleInChannel(uint8_t ch, const std::string &sample, int loops = 0);
            /// Plays a sample like in playSample(), but this time starts with volume 0 and
            /// increases it during `fadeIn` ms, doing a fade in effect. To make the same effect
            /// to stop a sample, see stopChannelWithFadeOut().
            int playSampleWithFadeIn(const std::string &sample, uint32_t fadeIn, int loops = 0);
            /// Same as playSampleWithFadeIn() but specifying the channel to play.
            bool playSampleWithFadeIn(uint8_t ch, const std::string &sample, uint32_t fadeIn, int loops = 0);
            /// Pauses a playing channel.
            void pauseChannel(uint8_t ch);
            /// Pauses all playing channels.
            void pauseAllChannels();
            /// Resumes a paused channel.
            void resumeChannel(uint8_t ch);
            /// Resumes all paused channels.
            void resumeAllChannels();
            /// Stops a channel from playing a sample.
            void stopChannel(uint8_t ch);
            /// Stops all channels that are playing a sample.
            void stopAllChannels();
            /// Stops a playing channel with a `ms` ms fade out effect.
            void stopChannelWithFadeOut(uint8_t channel, uint32_t ms);
            /// Stops all playing channels with a `ms` ms fade out effect.
            void stopAllChannelsWithFadeOut(uint32_t ms);
            /// Returns whether this channel is playing a sample or not.
            bool channelIsPlaying(uint8_t ch);
            /// Returns whether this channel is paused or not.
            bool channelIsPaused(uint8_t ch);

            /// Plays a music forever (by default) or a number of loops (0 is for 0 times, that
            /// differs with the playSample version). If a music was playing already, it will
            /// stop the previous and start the new. If the previous music is fading out, will
            /// wait until the end of it and then will start playing the new one.
            bool playMusic(const std::string &music, int loops = -1);
            /// Same as playMusic() but start withvolume 0 and increases it during `ms` ms, doing
            /// a fade in effect. To make the same effect when stopping, use stopMusicWithFadeOut().
            bool playMusicWithFadeIn(const std::string &music, uint32_t ms, int loops = -1);
            /// Same as playMusicWithFadeInt() but instead of starting at 0 units, starts
            /// somewhere you specify. The meaning of position is defined here:
            /// https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_65.html#SEC65
            bool playMusicWithFadeInStartingAt(const std::string &music, uint32_t ms, double position, int loops = -1);
            /// Gets the music volume.
            uint8_t getMusicVolume();
            /// Changes music volume. The minimum is 0 and the maximum is 128.
            void setMusicVolume(uint8_t volume);
            /// Pauses a playing music.
            void pauseMusic();
            /// Resumes a paused music.
            void resumeMusic();
            /// Rewind the music to the start. You can only do it if the music is of one of this
            /// types: MOD, OGG, MP3, Native MIDI.
            void rewindMusic();
            /// Changes the position of the playing music. The meaning of the argument is
            /// explained here: https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_65.html#SEC65
            void changePositionMusic(double pos);
            /// Stops the playing or paused music.
            void stopMusic();
            /// Stops a playing music with a `ms` ms fade out effect.
            bool stopMusicWithFadeOut(uint32_t ms);
            /// Returns whether there's music playing or not.
            bool musicIsPlaying();
            /// Returns whether there's a paused music or not.
            bool musicIsPaused();

            /// Applies a distance sound effect to a channel, where 0 is the closest position
            /// (and where the effect is disabled) and 255 the further position.
            bool setDistanceEffectOnChannel(uint8_t channel, uint8_t distance);
            /// Same as setDistanceEffectOnChannel(), but on all channels.
            bool setDistanceEffectOnAllChannels(uint8_t distance);
            /// Applies a position effect (emulating that something is in other place) using
            /// an degree angle and a distance (from 0 to 255). An angle of 0 and a distance of
            /// 0 disables the effect. Is not a good effect at all, but it's enough, we are not
            /// in 3D space :)
            bool setPositionEffectOnChannel(uint8_t channel, float angle, uint8_t distance);
            /// Same as setPositionEffectOnChannel() but for all channels. Why do you need this?
            bool setPositionEffectOnAllChannels(float angle, uint8_t distance);

            /// If you need to free some memory, you can delete samples from the loaded ones.
            void deleteSample(const std::string &sample);
            /// If you need to free some memory, you can delete musics from the loaded ones.
            void deleteMusic(const std::string &music);

        };

    private:
        SDL_Window* window;
        SDL_Renderer* renderer;
        std::string gamePath;
        TTF_Font* font = nullptr;
        Optional<Palette> palette;
        bool quit = false;
        std::map<std::string, Level*> levels;
        Level* currentLevel = nullptr, *nextCurrentLevel = nullptr;
        float scaleFactor = 1.0f;
        class retro::Timer* timerPtr = nullptr;
        CanvasMode mode;

        void importPaletteFromGimp(const std::string &path);
        void importPaletteFromPhotoshop(const std::string &path);
        void pollEvents(double&, std::function<void(bool)>);
        void updateObjects(Timer&);
        void deletePendingObjects();
        void parseCommands();

    protected:
        Logger &log;
        Audio audio;

        Game(const Builder &builder);

        /// Setup method.
        /**
         * Setup method, called when the game is preparing to start. Here you should
         * configure the game palette with setPalette() or importPalette(), the game font
         * with loadFont() and the levels with addLevel().
         **/
        virtual void setup() = 0;

        /// Cleanup method.
        /**
         * Called when the game is closing. Useful to cleanup stuff that were created before.
         */
        virtual void cleanup() = 0;

        /// Sets the palette for the whole game. Only one palette can be used in a game.
        template<class P>
        inline void setPalette(const P &p) {
            static_assert(std::is_base_of<Palette, P>::value, "Type must extend from Palette");
            this->palette = p;
        }
        /// Import a palette from a file that is available inside the game path. Supports Photoshop palettes or Gimp palettes.
        void importPalette(const std::string &path);
        void unsetPalette();
        /// Gets the current palette
        const Palette& getPalette();

        /// Loads a font to be used as a default font
        void loadFont(const std::string &str, size_t size = 8);

        /// Adds a level to the game. You can mark it as initial level.
        template<class LVL>
        LVL& addLevel(const char* name, bool initial = false);

    public:

        /// Gets the game path
        constexpr const std::string& getGamePath() const { return gamePath; }

        /// Get a level reference by name. Watch out if you set the name wrongly, your game will crash if it is.
        template<class LVL>
        LVL& getLevel(const char* name);

        /// Changes the current level to another one. Use correct names, or your game will crash.
        void changeLevel(const char* name);

        /// Starts the game itself: initializes some stuff and starts the game loop.
        void loop();
        /// Tells the game to stop the loop.
        void end();

        /// Opens a read stream to a file
        /**
         * Opens a file to be read, either in the current locale encoding (UTF-8 by default) or
         * in binary (not doing any kind of transformation). By default opens in binary. The file
         * must exist to be able to open it.
         * @param file Path to the file (from the game folder)
         * @param binary Sets whether the file is read in binary or as a text
         * @return InputFile with the new file opened or with an error
         * @see InputFile
         **/
        InputFile openReadFile(const std::string &file, bool binary = true) const;

        /// Opens a write stream to a file
        /**
         * Opens a file to be written, writing in the current locale encoding (UTF-8 by default) or
         * in binary (not doing any kind of transformation). By default writes in binary. If the
         * file exists, the file is emptied. But, if the file doesn't exist, it will be created.
         * If `append` is true and the file exists, instead of empty it, it will start writing at
         * the end of the file (appending to it).
         * @param file Path to the file (from the game folder)
         * @param binary Sets whether the file is writen in binary or as a text
         * @param append If the file exists, shall empty the contents of it (`false`) or append to it (`true`)
         * @return OutputFile with the new file opened or with an error
         * @see OutputFile
         */
        OutputFile openWriteFile(const std::string &file, bool binary = true, bool append = false) const;

        /// Opens a read/write stream to a file
        /**
         * Opens a file to be read and written, using the current locale encoding (UTF-8 by default)
         * or in binary (not doing any kind of transformation). By default, uses binary mode. If the
         * file doesn't exist, it will be created. If the file exists, won't do anything to it and will
         * start reading and writing from the beginning. If `append` is true, it will start reading and
         * writing from the end.
         * @param file Path to the file (from the game folder)
         * @param binary Sets whether the file is writen in binary or as a text
         * @param append If the file exists, shall start from the beginning (`false`) or from the end (`true`) of the file
         * @see InputOutputFile
         **/
        InputOutputFile openFile(const std::string &file, bool binary = true, bool append = false) const;

        /// Gets the reference to the audio system. If audio is disabled, anything you do
        /// won't have any effect (no surprise exceptions this case).
        Audio& getAudio() { return audio; }

        /// Enable capturing the mouse inside the window.
        void captureMouse(bool capture);

        /// Gets the Window object.
        Window getWindow();

        /// Gets the current level.
        const Level* getCurrentLevel() const;

        /// Gets the Timer reference used to count FPS, use it to know the FPS (instanteous and real). Use it only when the game has already started (when loop() has been called).
        const Timer& getTiming() const;

        /// Saves the game status into a file.
        /**
         * The status stores instance attributes of the objects of the every level created in
         * the game into a JSON file. But it won't save the objects created in a level, so you
         * must create them before restore the status (applicable when you create objects in a
         * level depending on the state). If you are in this sittuation, you can read the stored
         * values and create the objects and then call the super implementation.
         * @param saveName Name of the saved status.
         **/
        void saveGame(const char* saveName);

        /// Restores the game status into a file.
        void restoreGame(const char* saveName);

        /// Stops the game loop and closes everything
        constexpr void closeGame() { quit = true; }

        virtual ~Game();

        friend GameActions;
        friend Level;
        friend Map;
        friend Sprites;
        friend UIObject;
        friend Image;

    };

    template<class GameClass>
    GameClass* Game::Builder::build() {
        static_assert(std::is_base_of<Game, GameClass>::value, "Type must extend from Game");
        return new GameClass(*this);
    }

}


#include <Level.hpp>

namespace retro {

    template<class LVL>
    LVL& Game::addLevel(const char* name, bool initial) {
        static_assert(std::is_base_of<Level, LVL>::value, "Type must extend from Level");
        this->levels[name] = new LVL(*this, name);
        if(initial) this->currentLevel = this->levels[name];
        log.debug("Added level %s", name);
        return *dynamic_cast<LVL*>(this->levels[name]);
    }

    template<class LVL>
    LVL& Game::getLevel(const char* name) {
        static_assert(std::is_base_of<Level, LVL>::value, "Type must extend from Level");
        if(this->levels.find(name) != this->levels.end()) {
            return *dynamic_cast<LVL*>(this->levels[name]);
        } else {
            throw "Level not found";
        }
    }

}
