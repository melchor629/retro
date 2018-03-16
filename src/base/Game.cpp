#include <Game.hpp>
#include <stdexcept>
#include <cmath>
#include <chrono>
#include <Timer.hpp>
#include <Level.hpp>
#include <MapObject.hpp>
#include <Platform.hpp>

#if !defined(_WIN32) and !defined(__ANDROID__) and !defined(__IOS__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#if defined(__APPLE__) && defined(__MACH__) and !defined(__IOS__)
#include <SDL2_ttf/SDL_ttf.h>
#include <SDL2_mixer/SDL_mixer.h>
#else
#if !defined(_WIN32) and !defined(__ANDROID__) and !defined(__IOS__)
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL_video.h>
#else
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#endif
#endif

using namespace retro;
using namespace std;
using namespace glm;


void retro::to_json(json &j, const Level &level) {
   level.saveState(j);
}

void retro::from_json(const json &j, Level &level) {
   level.restoreState(j);
}

static volatile bool sdl_init = false;

Game::Builder::Builder() {
    if(!sdl_init) {
        if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
            throw runtime_error(string("Could not initialize SDL: ") + SDL_GetError());
        }
        sdl_init = true;
    }

    this->frame.pos = { SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED };
    this->frame.size = { 1280, 720 };
    this->visible = true;
#ifndef DEFAULT_GAME_PATH
#ifndef _WIN32
#ifdef __ANDROID__
    this->gamePath = "res/";
#elif defined(__linux__)
    this->gamePath = getCurrentDirectory() + "/../share/retro++/res/";
#elif defined(__IOS__)
    this->gamePath = "";
#else
    this->gamePath = getCurrentDirectory() + "/Resources/";
#endif
#else
    this->gamePath = getCurrentDirectory() + "/res/";
#endif
#else
#define STRINGIFY(d) #d
#define STR(d) STRINGIFY(d)
    this->gamePath = STR(DEFAULT_GAME_PATH);
    this->gamePath += "/";
#endif
}

Game::Builder& Game::Builder::setFrame(const Frame &frame) {
    this->frame = frame;
    return *this;
}

Game::Builder& Game::Builder::setPosition(const uvec2 &pos) {
    this->frame.pos = pos;
    return *this;
}

Game::Builder& Game::Builder::setSize(uint32_t width, uint32_t height) {
    this->frame.size = { width, height };
    return *this;
}

Game::Builder& Game::Builder::setName(const string &name) {
    this->name = name;
    return *this;
}

Game::Builder& Game::Builder::setFullscreen(DisplayMode dp, bool fs) {
    this->dp = dp;
    this->dp_flag = fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
    this->frame.size.x = float(dp.width);
    this->frame.size.y = float(dp.height);
    return *this;
}

Game::Builder& Game::Builder::setVisible(bool visible) {
    this->visible = visible;
    return *this;
}

Game::Builder& Game::Builder::setResizable(bool resizable) {
    this->resizable = resizable;
    return *this;
}

Game::Builder& Game::Builder::setGamePath(std::string gamePath) {
    this->gamePath = gamePath;
    return *this;
}

Game::Builder& Game::Builder::enableAudio(int sampleRate, int channels, int audioChunkSize) {
    this->sampleRate = sampleRate;
    this->channels = channels;
    this->audioChunkSize = audioChunkSize;
    return *this;
}

Game::Builder& Game::Builder::changeCanvasMode(Game::CanvasMode mode) {
    this->canvasMode = mode;
    return *this;
}

Optional<DisplayMode> Game::Builder::getDisplayMode(int monitor, int mode) {
    if(monitor < SDL_GetNumVideoDisplays()) {
        if(mode < SDL_GetNumDisplayModes(monitor)) {
            SDL_DisplayMode dm;
            SDL_GetDisplayMode(monitor, mode, &dm);
            return DisplayMode{
                dm.format,
                (uint32_t) dm.w,
                (uint32_t) dm.h,
                (uint32_t) dm.refresh_rate,
                dm.driverdata
            };
        }
    }
    return {};
}

Optional<DisplayMode> Game::Builder::getCurrentDisplayMode() {
    SDL_DisplayMode displayMode;
    if(SDL_GetCurrentDisplayMode(0, &displayMode) == 0) {
        return DisplayMode{
            displayMode.format,
            (uint32_t) displayMode.w,
            (uint32_t) displayMode.h,
            (uint32_t) displayMode.refresh_rate,
            displayMode.driverdata
        };
    }
    return {};
}

///////////////////////////////////////////////////////////////////////////////////////////////

Game::Window::Window(SDL_Window* window, Logger &log, const float &scaleFactor): window(window), scaleFactor(scaleFactor), log(log) {}

bool Game::Window::isResizable() {
    return SDL_GetWindowFlags(window) & SDL_WINDOW_RESIZABLE;
}

void Game::Window::setResizable(bool v) {
#ifndef SDL2_OLD
    SDL_SetWindowResizable(window, v ? SDL_TRUE : SDL_FALSE);
#endif
}

void Game::Window::setIcon(void* pixels, const uvec2 &size, uint8_t bitDepth, uint8_t channels) {
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(pixels,
                                                    size.x,
                                                    size.y,
                                                    bitDepth,
                                                    size.x * bitDepth / 8 * channels,
                                                    0x000000ff,
                                                    0x0000ff00,
                                                    0x00ff0000,
                                                    (channels == 3) ? 0 : 0xff000000);
    SDL_SetWindowIcon(window, surface);
    SDL_FreeSurface(surface);
#if defined(__APPLE__) && defined(__MACH__)
    #include "TargetConditionals.h"
#if !defined(TARGET_OS_IPHONE) and !defined(TARGET_IPHONE_SIMULATOR)
    changeDockIcon(pixels, size.x, size.y);
#endif
#endif
}

uvec2 Game::Window::getSize() {
    uvec2 size;
    SDL_GetWindowSize(window, reinterpret_cast<int*>(&size.x), reinterpret_cast<int*>(&size.y));
    return size;
}

void Game::Window::setSize(const uvec2 &size) {
    SDL_SetWindowSize(window, size.x, size.y);
}

float Game::Window::getOpacity() {
#ifndef SDL2_OLD
    float f;
    return SDL_GetWindowOpacity(window, &f) == 0 ? f : 1.0f;
#else
    return 1.0f;
#endif
}

void Game::Window::setOpacity(float op) {
#ifndef SDL2_OLD
    if(SDL_SetWindowOpacity(window, op) == -1) {
        log.error("Could not change the window opacity: %s", SDL_GetError());
    }
#endif
}

bool Game::Window::isBordered() {
    return !(SDL_GetWindowFlags(window) & SDL_WINDOW_BORDERLESS);
}

void Game::Window::setBordered(bool v) {
    SDL_SetWindowBordered(window, v ? SDL_TRUE : SDL_FALSE);
}

ivec2 Game::Window::getPosition() {
    ivec2 pos;
    SDL_GetWindowPosition(window, &pos.x, &pos.y);
    return pos;
}

void Game::Window::setPosition(const ivec2 &pos) {
    SDL_SetWindowPosition(window, pos.x, pos.y);
}

uvec2 Game::Window::getMinimumSize() {
    uvec2 size;
    SDL_GetWindowMinimumSize(window, reinterpret_cast<int*>(&size.x), reinterpret_cast<int*>(&size.y));
    return size;
}

void Game::Window::setMinimumSize(const uvec2 &size) {
    SDL_SetWindowMinimumSize(window, size.x, size.y);
}

uvec2 Game::Window::getMaximumSize() {
    uvec2 size;
    SDL_GetWindowMaximumSize(window, reinterpret_cast<int*>(&size.x), reinterpret_cast<int*>(&size.y));
    return size;
}

void Game::Window::setMaximumSize(const uvec2 &size) {
    SDL_SetWindowMaximumSize(window, size.x, size.y);
}

bool Game::Window::isFullscreenMode() {
    return SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN && !isWindowedFullscreenMode();
}

bool Game::Window::isWindowedFullscreenMode() {
    return SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP;
}

bool Game::Window::isWindowedMode() {
    return !isFullscreenMode() && !isWindowedFullscreenMode();
}

bool Game::Window::changeToFullscreenMode() {
    return SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN) == 0;
}

bool Game::Window::changeToWindowedFullscreenMode() {
    return SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP) == 0;
}

bool Game::Window::changeToWindowedMode() {
    return SDL_SetWindowFullscreen(window, 0) == 0;
}

const char* Game::Window::getTitle() {
    return SDL_GetWindowTitle(window);
}

void Game::Window::setTitle(const std::string &title) {
    SDL_SetWindowTitle(window, title.c_str());
}

Optional<DisplayMode> Game::Window::getDisplayMode(int monitor, int mode) {
    if(monitor < SDL_GetNumVideoDisplays()) {
        if(mode < SDL_GetNumDisplayModes(monitor)) {
            SDL_DisplayMode dm;
            SDL_GetDisplayMode(monitor, mode, &dm);
            return DisplayMode{dm.format, (uint32_t) dm.w, (uint32_t) dm.h, (uint32_t) dm.refresh_rate, dm.driverdata };
        }
    }
    return {};
}

Optional<DisplayMode> Game::Window::getCurrentDisplayMode() {
    SDL_DisplayMode mode;
    if(SDL_GetWindowDisplayMode(window, &mode) == 0) {
        return DisplayMode{ mode.format, (uint32_t) mode.w, (uint32_t) mode.h, (uint32_t) mode.refresh_rate, mode.driverdata };
    } else {
        log.error("Could not get current display mode: %s", SDL_GetError());
        return {};
    }
}

bool Game::Window::setCurrentDisplayMode(const DisplayMode &dp) {
    SDL_DisplayMode mode{ dp.format, (int) dp.width, (int) dp.height, (int) dp.refreshRate, dp.driverData };
    if(SDL_SetWindowDisplayMode(window, &mode) == 0) {
        return true;
    } else {
        log.error("Could not change display mode: %s", SDL_GetError());
        return false;
    }
}

void Game::Window::show() {
    SDL_ShowWindow(window);
}

void Game::Window::hide() {
    SDL_HideWindow(window);
}

bool Game::Window::isHidden() {
    return (bool) (SDL_GetWindowFlags(window) & SDL_WINDOW_HIDDEN);
}

float Game::Window::getScaleFactor() {
    return scaleFactor;
}

///////////////////////////////////////////////////////////////////////////////////////////////

Game::Audio::Audio(Logger &log, bool enabled, const string &gp): log(log), enabled(enabled), gamePath(gp) {}

Game::Audio::~Audio() {
    for(auto &pair: samples) {
        Mix_FreeChunk(pair.second);
    }

    for(auto &pair: musics) {
        Mix_FreeMusic(pair.second);
    }
}

const char* Game::Audio::getLastError() {
    return Mix_GetError();
}

void Game::Audio::loadSample(const string &path) {
    if(!enabled) return;
    string name = path.substr(0, path.rfind("."));
    if(name.find("/") != string::npos) name = name.substr(name.rfind("/") + 1);
    loadSample(path, name);
}

void Game::Audio::loadSample(const string &path, const string &name) {
    if(!enabled) return;
    auto it = samples.find(name);
    if(it != samples.end()) {
        Mix_FreeChunk(it->second);
    }

    string fullPath = gamePath + path;
    Mix_Chunk* chunk = Mix_LoadWAV(fullPath.c_str());
    if(chunk == nullptr) throw runtime_error(string("Cannot load sample: ") + Mix_GetError());
    log.debug("Loaded sample '%s' from file %s", name.c_str(), path.c_str());
    samples[name] = chunk;
}

void Game::Audio::loadSample(void* memory, const string &name) {
    if(!enabled) return;
    auto it = samples.find(name);
    if(it != samples.end()) {
        Mix_FreeChunk(it->second);
    }

    Mix_Chunk* chunk = Mix_QuickLoad_WAV(static_cast<uint8_t*>(memory));
    if(chunk == nullptr) throw runtime_error(string("Cannot load sample: ") + Mix_GetError());
    log.debug("Loaded sample '%s' from memory", name.c_str());
    samples[name] = chunk;
}

void Game::Audio::loadMusic(const string &path) {
    if(!enabled) return;
    string name = path.substr(0, path.rfind("."));
    if(name.find("/") != string::npos) name = name.substr(name.rfind("/") + 1);
    loadMusic(path, name);
}

void Game::Audio::loadMusic(const string &path, const string &name) {
    if(!enabled) return;
    auto it = musics.find(name);
    if(it != musics.end()) {
        Mix_FreeMusic(it->second);
    }

    string fullPath = gamePath + path;
    Mix_Music* music = Mix_LoadMUS(fullPath.c_str());
    if(music == nullptr) throw runtime_error(string("Cannot load music: ") + Mix_GetError());
    log.debug("Loaded music '%s' from file %s", name.c_str(), path.c_str());
    musics[name] = music;
}

void Game::Audio::changeNumberOfChannels(uint8_t num) {
    if(!enabled) return;
    log.debug("Changed number of sample channels to %d (%d requested)", Mix_AllocateChannels(num), num);
}

void Game::Audio::setChannelVolume(uint8_t ch, uint8_t volume) {
    if(!enabled) return;
    log.debug("Changed volume of channel %d to %u (%u requiested)", ch, Mix_Volume(ch, volume), volume);
}

uint8_t Game::Audio::getChannelVolume(uint8_t ch) {
    if(!enabled) return 0;
    else return Mix_Volume(ch, -1);
}

int Game::Audio::playSample(const string &sample, int loops) {
    if(!enabled) return -2;
    return Mix_PlayChannel(-1, findChunk(sample), loops);
}

bool Game::Audio::playSampleInChannel(uint8_t ch, const string &sample, int loops) {
    if(!enabled) return 0;
    return Mix_PlayChannel(ch, findChunk(sample), loops) == ch;
}

int Game::Audio::playSampleWithFadeIn(const string &sample, uint32_t fadeIn, int loops) {
    if(!enabled) return -2;
    return Mix_FadeInChannel(-1, findChunk(sample), loops, fadeIn);
}

bool Game::Audio::playSampleWithFadeIn(uint8_t ch, const string &sample, uint32_t fadeIn, int loops) {
    if(!enabled) return false;
    return Mix_FadeInChannel(ch, findChunk(sample), loops, fadeIn) == ch;
}

void Game::Audio::pauseChannel(uint8_t ch) {
    if(!enabled) return;
    Mix_Pause(ch);
}

void Game::Audio::pauseAllChannels() {
    if(!enabled) return;
    Mix_Pause(-1);
}

void Game::Audio::resumeChannel(uint8_t ch) {
    if(!enabled) return;
    Mix_Resume(ch);
}

void Game::Audio::resumeAllChannels() {
    if(!enabled) return;
    Mix_Resume(-1);
}

void Game::Audio::stopChannel(uint8_t ch) {
    if(!enabled) return;
    Mix_HaltChannel(ch);
}

void Game::Audio::stopAllChannels() {
    if(!enabled) return;
    Mix_HaltChannel(-1);
}

void Game::Audio::stopChannelWithFadeOut(uint8_t channel, uint32_t ms) {
    if(!enabled) return;
    Mix_FadeOutChannel(channel, ms);
}

void Game::Audio::stopAllChannelsWithFadeOut(uint32_t ms) {
    if(!enabled) return;
    Mix_FadeOutChannel(-1, ms);
}

bool Game::Audio::channelIsPlaying(uint8_t ch) {
    if(!enabled) return false;
    return Mix_Playing(ch);
}

bool Game::Audio::channelIsPaused(uint8_t ch) {
    if(!enabled) return false;
    return Mix_Paused(ch);
}

bool Game::Audio::playMusic(const string &music, int loops) {
    if(!enabled) return false;
    return Mix_PlayMusic(findMusic(music), loops);
}

bool Game::Audio::playMusicWithFadeIn(const string &music, uint32_t ms, int loops) {
    if(!enabled) return false;
    return Mix_FadeInMusic(findMusic(music), loops, ms);
}

bool Game::Audio::playMusicWithFadeInStartingAt(const string &music, uint32_t ms, double position, int loops) {
    if(!enabled) return false;
    return Mix_FadeInMusicPos(findMusic(music), loops, ms, position);
}

uint8_t Game::Audio::getMusicVolume() {
    if(!enabled) return 0;
    return Mix_VolumeMusic(-1);
}

void Game::Audio::setMusicVolume(uint8_t volume) {
    if(!enabled) return;
    Mix_VolumeMusic(volume);
}

void Game::Audio::pauseMusic() {
    if(!enabled) return;
    Mix_PauseMusic();
}

void Game::Audio::resumeMusic() {
    if(!enabled) return;
    Mix_ResumeMusic();
}

void Game::Audio::rewindMusic() {
    if(!enabled) return;
    Mix_RewindMusic();
}

void Game::Audio::changePositionMusic(double pos) {
    if(!enabled) return;
    Mix_SetMusicPosition(pos);
}

void Game::Audio::stopMusic() {
    if(!enabled) return;
    Mix_HaltMusic();
}

bool Game::Audio::stopMusicWithFadeOut(uint32_t ms) {
    if(!enabled) return false;
    return Mix_FadeOutMusic(ms) == 0;
}

bool Game::Audio::musicIsPlaying() {
    if(!enabled) return false;
    return Mix_PlayingMusic() == 0;
}

bool Game::Audio::musicIsPaused() {
    if(!enabled) return false;
    return Mix_PausedMusic() == 0;
}

bool Game::Audio::setDistanceEffectOnChannel(uint8_t channel, uint8_t distance) {
    if(!enabled) return false;
    return Mix_SetDistance(channel, distance) != 0;
}

bool Game::Audio::setDistanceEffectOnAllChannels(uint8_t distance) {
    if(!enabled) return false;
    return Mix_SetDistance(-1, distance) != 0;
}

bool Game::Audio::setPositionEffectOnChannel(uint8_t channel, float angle, uint8_t distance) {
    if(!enabled) return false;
    return Mix_SetPosition(channel, int16_t(fmod(angle, 360.0f)), distance) != 0;
}

bool Game::Audio::setPositionEffectOnAllChannels(float angle, uint8_t distance) {
    if(!enabled) return false;
    return Mix_SetPosition(-1, int16_t(fmod(angle, 360.0f)), distance) != 0;
}

void Game::Audio::deleteSample(const string &sample) {
    if(!enabled) return;
    auto it = samples.find(sample);
    if(it != samples.end()) {
        Mix_FreeChunk(it->second);
        samples.erase(it);
    }
}

void Game::Audio::deleteMusic(const string &music) {
    if(!enabled) return;
    auto it = musics.find(music);
    if(it != musics.end()) {
        Mix_FreeMusic(it->second);
        musics.erase(it);
    }
}

Mix_Chunk* Game::Audio::findChunk(const std::string &sample) {
    auto it = samples.find(sample);
    if(it == samples.end()) throw runtime_error("Sample named '" + sample + "' not found");
    return it->second;
}

Mix_Music* Game::Audio::findMusic(const std::string &music) {
    auto it = musics.find(music);
    if(it == musics.end()) throw runtime_error("Music named '" + music + "' not found");
    return it->second;
}


///////////////////////////////////////////////////////////////////////////////////////////////


Game::Game(const Game::Builder &builder): log(Logger::getLogger(builder.name)), audio(log, builder.sampleRate != 0, gamePath) {
    srand(time(NULL));
#ifndef __ANDROID__
    this->window = SDL_CreateWindow(
        builder.name.c_str(),
        builder.frame.pos.x,
        builder.frame.pos.y,
        builder.frame.size.x,
        builder.frame.size.y,
        (builder.visible ? SDL_WINDOW_SHOWN : SDL_WINDOW_HIDDEN) | (builder.dp ? builder.dp_flag : 0) | SDL_WINDOW_OPENGL | (builder.resizable ? SDL_WINDOW_RESIZABLE : 0)
    );
    if(this->window == nullptr) {
        throw runtime_error(string("Could not create the window: ") + SDL_GetError());
    }
    
    log.debug("Created window at (%.0f, %.0f) with size (%.0f, %.0f) %svisible%s",
              builder.frame.pos.x,
              builder.frame.pos.y,
              builder.frame.size.x,
              builder.frame.size.y,
              builder.visible ? "" : "in",
              builder.resizable ? " and resizable" : ""
    );
#else
    this->window = SDL_CreateWindow(
            builder.name.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            builder.frame.size.x,
            builder.frame.size.y,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );
    if(this->window == nullptr) {
        throw runtime_error(string("Could not create the window: ") + SDL_GetError());
    }

    log.debug("Created window with size (%.0f, %.0f)",
              builder.frame.size.x,
              builder.frame.size.y
    );
#endif

    this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    if(this->renderer == nullptr) {
        throw runtime_error(string("Could not create the renderer: ") + SDL_GetError());
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); //Pixel Art :)
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1"); //VSync hermano

    if(TTF_Init() != 0) {
        throw runtime_error(string("Could not initialize SDL_ttf: ") + TTF_GetError());
    }

    if(builder.sampleRate != 0) {
        if(Mix_OpenAudio(builder.sampleRate, AUDIO_S16, builder.channels, builder.audioChunkSize) != 0) {
            throw runtime_error(string("Could not initialize SDL_mixer: ") + Mix_GetError());
        }

        int size = Mix_GetNumChunkDecoders();
        for(int i = 0; i < size; i++) {
            log.debug("Available chunk decoder #%d: %s", i, Mix_GetChunkDecoder(i));
        }

        size = Mix_GetNumMusicDecoders();
        for(int i = 0; i < size; i++) {
            log.debug("Available music decoder #%d: %s", i, Mix_GetMusicDecoder(i));
        }

        audio.changeNumberOfChannels(8);
    }

    this->mode = builder.canvasMode;
    this->gamePath = builder.gamePath;
    log.info("Using %s as game path", gamePath.c_str());
}

void Game::pollEvents(double &fpslimit, function<void(bool)> resize) {
    SDL_Event e;
    while(SDL_PollEvent(&e) != 0) {
        if(e.type == SDL_QUIT) {
            this->end();
        } else if(e.type == SDL_KEYDOWN) {
            currentLevel->keyDown(e.key.keysym.scancode);
        } else if(e.type == SDL_KEYUP) {
            currentLevel->keyUp(e.key.keysym.scancode);
        } else if(e.type == SDL_MOUSEBUTTONDOWN) {
            currentLevel->mouseDown(e.button.button, e.button.clicks);
        } else if(e.type == SDL_MOUSEBUTTONUP) {
            currentLevel->mouseUp(e.button.button, e.button.clicks);
        } else if(e.type == SDL_MOUSEMOTION) {
            currentLevel->mouseMoved({ e.motion.x / 10 / scaleFactor, e.motion.y / 10 / scaleFactor }, { e.motion.xrel / 10.f / scaleFactor, e.motion.yrel / 10.f / scaleFactor });
        } else if(e.type == SDL_MOUSEWHEEL) {
            currentLevel->mouseWheelMoved({ e.wheel.x, e.wheel.y });
        } else if(e.type == SDL_TEXTINPUT) {
            currentLevel->keyText(e.text.text);
        } else if(e.type == SDL_TEXTEDITING) {
            currentLevel->keyTextEdit(e.edit.text, e.edit.start, e.edit.length);
        } else if(e.type == SDL_WINDOWEVENT) {
            //This way, will limit FPS when the window is unfocused or minimized
            //On some platforms, the VSync is lost when one of the upper situations
            //is happening, so to avoid a high CPU usage, will limit the number of renders
            if(e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                fpslimit = 1.0 / 144.0;
                log.debug("FPS limit set to 144");
            } else if(e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                fpslimit = 1.0 / 5.0;
                log.debug("FPS limit set to 5");
            } else if(e.window.event == SDL_WINDOWEVENT_MINIMIZED) {
                fpslimit = 1.0;
                log.debug("FPS limit set to 1");
            } else if(e.window.event == SDL_WINDOWEVENT_RESIZED) {
                resize(true);
            } else if(e.window.event == SDL_WINDOWEVENT_EXPOSED) {
                currentLevel->mustRedraw();
                log.debug("Window must be redrawn");
            }
        }
    }
}

void Game::updateObjects(Timer &timer) {
    if(currentLevel->preupdate(timer.getDelta())) {
        for(Object *obj : currentLevel->objects) {
            if(!obj->isDisabled()) {
                if(dynamic_cast<Player*>(obj) != nullptr) {
                    Player* player = (Player*) obj;
                    for(Object *obj : currentLevel->objects) {
                        if(player != obj && dynamic_cast<Collisionable*>(obj) != nullptr) {
                            player->checkCollision(*dynamic_cast<Collisionable*>(obj));
                        } else if(dynamic_cast<MapObject*>(obj) != nullptr) {
                            auto frame = player->nextFrame(timer.getDelta());
                            MapObject* map = (MapObject*) obj;
                            if(!map->validPosition({ frame.pos.x + frame.size.x / 2, frame.pos.y })) {
                                player->collisionWithMap(TOP);
                            } if(!map->validPosition({ frame.pos.x + frame.size.x / 2, frame.pos.y + frame.size.y })) {
                                player->collisionWithMap(BOTTOM);
                            } if(!map->validPosition({ frame.pos.x, frame.pos.y + frame.size.y / 2 })) {
                                player->collisionWithMap(LEFT);
                            } if(!map->validPosition({ frame.pos.x + frame.size.x, frame.pos.y + frame.size.y / 2 })) {
                                player->collisionWithMap(RIGHT);
                            }
                        }
                    }
                }
                obj->update(timer.getDelta(), currentLevel->ga);
            }
        }
        currentLevel->update(timer.getDelta());
    }
}

void Game::deletePendingObjects() {
    for(auto* &o: currentLevel->pendingToDeleteObjects) {
        auto it = find_if(
            currentLevel->objects.begin(),
            currentLevel->objects.end(),
            [&o] (auto* &it) { return it == o; }
        );
        if(it != currentLevel->objects.end()) {
            currentLevel->log.debug("Deleted %s object", (*it)->getName());
            delete *it;
            currentLevel->objects.erase(it);
        } else {
            auto it = find_if(
                currentLevel->uiObjects.begin(),
                currentLevel->uiObjects.end(),
                [&o] (auto* &it) { return it == o; }
            );
            if(it != currentLevel->uiObjects.end()) {
                currentLevel->log.debug("Deleted %s UI object", (*it)->getName());
                delete *it;
                currentLevel->uiObjects.erase(it);
            }
        }
    }
    currentLevel->pendingToDeleteObjects.clear();
}

void Game::parseCommands() {
    static auto split = [] (string cmd, auto delim) -> vector<string> {
        vector<string> path;
        size_t pos;
        while((pos = cmd.find(delim)) != string::npos) {
            path.push_back(cmd.substr(0, pos));
            cmd = cmd.substr(pos + strlen(delim));
        }
        if(!cmd.empty()) path.push_back(cmd);
        return path;
    };

    static auto exec = [] (json &resp, json &j, vector<string> &attr, Optional<json> value) {
        function<void(json&,json&,vector<string>&,Optional<json>)> rec;
        auto typeStr = [] (json::value_t v) -> string {
            using type = nlohmann::detail::value_t;
            switch(v) {
                case type::null: return "null";
                case type::object: return "Object";
                case type::array: return "Array";
                case type::string: return "String";
                case type::boolean: return "Bool";
                case type::number_integer: return "Int";
                case type::number_unsigned: return "UInt";
                case type::number_float: return "Float";
                case type::discarded: return "?";
                default: return "¿";
            }
        };
        rec = [&rec, &typeStr] (json &resp, json &json, vector<string> &attr, Optional<nlohmann::json> value) {
            auto checc = [&attr, &typeStr, &resp] (nlohmann::json &a, const nlohmann::json &b) -> bool {
                if(a.type() == b.type()) {
                    a = b;
                    return true;
                } else if(a.is_number() && b.is_number() && a.is_number_unsigned() && b.is_number_integer() && b.get<int64_t>() >= 0) {
                    a = uint64_t(b.get<int64_t>());
                    return true;
                } else if(a.is_number() && b.is_number() && a.is_number_integer() && b.is_number_unsigned() && b.get<uint64_t>() < INT64_MAX) {
                    a = int64_t(b.get<uint64_t>());
                    return true;
                } else {
                    resp["error"] = "'" + attr[0] + "' attribute's type is " + typeStr(a.type()) +
                        ", but value has a different type, " + typeStr(b.type());
                    return false;
                }
            };

            if(!json.is_null() && json.is_primitive()) {
                if(attr.size() == 1) {
                    if(value && !checc(json, *value)) return;
                    resp = json;
                } else {
                    resp["error"] = "Attribute '" + attr[0] + "' is a " + typeStr(json);
                }
            } else if(json.is_array()) {
                if(attr.size() <= 1) {
                    if(!value) {
                        resp["elements"] = json.size();
                        resp["values"] = json;
                    } else {
                        resp["error"] = "Cannot set on an array. Modify every item one by one";
                    }
                } else {
                    try {
                        size_t i = std::stoul(attr[1]);
                        if(i < json.size()) {
                            attr.erase(attr.begin());
                            rec(resp, json[i], attr, value);
                        } else {
                            resp["error"] = "Position '" + attr[1] + "' is not inside the array '" + attr[0] + "'";
                        }
                    } catch(...) {
                        resp["error"] = "Attribute '" + attr[0] + "' is an array and '" + attr[1] + "' is not a valid position";
                    }
                }
            } else if(json.is_object()) {
                if(attr.size() <= 1) {
                    if(!value) {
                        resp["options"] = json::array();
                        for(auto &pair: json.items()) {
                            resp["options"].push_back({ { "attribute", pair.key() }, { "type", typeStr(pair.value().type()) } });
                        }
                        resp["values"] = json;
                    } else {
                        for(auto &pair: json.items()) {
                            auto it = value->find(pair.key());
                            if(it != value->end()) {
                                vector<string> jaj = { pair.key() };
                                rec(resp, json[pair.key()], jaj, Optional<nlohmann::json>(*it));
                            }
                        }
                        resp = json;
                    }
                } else if(json.find(attr[1]) != json.end()) {
                    attr.erase(attr.begin());
                    rec(resp, json[attr[0]], attr, value);
                } else {
                    resp["error"] = "Attribute '" + attr[1] + "' is not inside the object '" + attr[0] + "'";
                }
            } else if(json.is_null()) {
                if(value) json = *value;
                resp = json;
            } else {
                resp["error"] = { "WTF!?", attr[0], typeStr(json.type()) };
            }
        };
        rec(resp, j, attr, value);
    };

    auto cmd = getCommand();
    if(cmd) {
        if(!cmd->data.is_array()) {
            sendCommandResponse(*cmd, { { "error", "Request must be an array" } });
            return;
        }
        json resp = json::array();
        if(cmd->data.size() == 0) {
            log.debug("Received empty command");
            resp[0]["options"] = { { { "attribute", "game" }, { "type", "Object" } } };
        }
        for(size_t ir = 0; ir < cmd->data.size(); ir++) {
            string cmdStr;
            if(!cmd->data[ir].is_object()) {
                sendCommandResponse(*cmd, { { "error", "Command [" + to_string(ir) + "] is not an object" } });
                return;
            } else if(!cmd->data[ir]["command"].is_string() && !cmd->data[ir]["command"].is_null()) {
                sendCommandResponse(*cmd, { { "error", "Command [" + to_string(ir) + "].command is not a string" } });
                return;
            } else if(cmd->data[ir]["command"].is_null() || (cmdStr = cmd->data[ir]["command"]).empty()) {
                log.debug("Received empty command");
                resp[ir]["options"] = { { { "attribute", "game" }, { "type", "Object" } } };
            } else {
                Optional<json> value = !cmd->data[ir]["value"].is_null() ? Optional<json>(cmd->data[ir]["value"]) : Optional<json>{};
                auto attribute = split(cmdStr, "::");
                if(value) log.debug("Received command '%s' with argument '%s'", cmdStr.c_str(), value->dump().c_str());
                else log.debug("Received command '%s'", cmdStr.c_str());
                if(attribute[0] == "game") {
                    if(attribute.size() == 1) {
                        resp[ir] = {{ "options",
                            {
                                { { "attribute", "currentLevel" }, { "type", "Object" } },
                                { { "attribute", "levels" }, { "type", "Array" } },
                                { { "attribute", "name" }, { "type", "String" } },
                                { { "attribute", "path" }, { "type", "String" } },
                                { { "attribute", "quit" }, { "type", "Bool" } }
                            }
                        }};
                    } else if(attribute[1] == "currentLevel") {
                        json j;
                        auto nattr = attribute;
                        nattr.erase(nattr.begin());
                        currentLevel->saveState(j);
                        exec(resp[ir], j, nattr, value);
                        if(value) currentLevel->restoreState(j);
                    } else if(attribute[1] == "levels") {
                        if(attribute.size() > 2) {
                            auto it = levels.find(attribute[2]);
                            if(it != levels.end()) {
                                json j;
                                currentLevel->saveState(j);
                                auto nattr = attribute;
                                nattr.erase(nattr.begin());
                                nattr.erase(nattr.begin());
                                exec(resp[ir], j, nattr, value);
                                if(value) currentLevel->restoreState(j);
                            } else {
                                resp[ir]["error"] = "Level '" + attribute[2] + "' not found";
                            }
                        } else {
                            resp[ir] = { { "levels", json::array() } };
                            for(auto &level: levels) {
                                resp[ir]["levels"].push_back(level.first);
                            }
                        }
                    } else if(attribute[1] == "name") {
                        if(attribute.size() > 2) {
                            resp[ir]["error"] = "Type of game::name is string, not object";
                        } else {
                            resp[ir] = getWindow().getTitle();
                        }
                    } else if(attribute[1] == "path") {
                        if(attribute.size() > 2) {
                            resp[ir]["error"] = "Type of game::path is string, not object";
                        } else {
                            resp[ir] = gamePath;
                        }
                    } else if(attribute[1] == "quit" && attribute.size() == 2) {
                        quit = true;
                        resp[ir] = "true";
                    } else {
                        resp[ir]["error"] = "Undefined attribute '" + attribute[1] + "'";
                    }
                } else {
                    resp[ir]["error"] = "Undefined attribute '" + attribute[0] + "'";
                }
            }
        }
        sendCommandResponse(*cmd, resp);
    }
}


class UILevel: public Level {
    void setup() override {}
    void update(float) override {}
    void draw() override {}
public:
    UILevel(Game &g): Level(g, "UILevel") {}
};

#ifdef __ANDROID__
namespace retro {
    extern float _android_factor_scale;
}
#endif
void textCache_collect_garbage();
void textCache_clear_all_entries();
void Game::loop() {
    Timer timer;
    timerPtr = &timer;

    this->setup();

    if(this->currentLevel == nullptr)
        throw runtime_error("No initial level has been selected");
    this->currentLevel->setup();

    SDL_SetRenderDrawBlendMode(this->renderer, SDL_BlendMode::SDL_BLENDMODE_BLEND);

    ivec2 size, wsize, canvasSize;
    SDL_Texture* rendererTexture;
    auto resizeFunc = [this, &size, &wsize, &rendererTexture, &canvasSize] (bool f) {
        const glm::vec2 oldSize = { size.x / 10 / scaleFactor, size.y / 10 / scaleFactor };
        if(f) SDL_DestroyTexture(rendererTexture);
        SDL_GetRendererOutputSize(this->renderer, &size.x, &size.y);
        SDL_GetWindowSize(this->window, &wsize.x, &wsize.y);
#ifndef __ANDROID__
        this->scaleFactor = float(size.x) / float(wsize.x);
#else
        this->scaleFactor = _android_factor_scale;
#endif
        auto r = double(size.x) / double(size.y);
        if(mode == CanvasMode::FreeMode) {
            canvasSize = { size.x / 10, size.y / 10 };
        } else {
            canvasSize = { float(mode), float(mode) / r };
        }
        rendererTexture = SDL_CreateTexture(this->renderer,
                                            SDL_PIXELFORMAT_RGBA8888,
                                            SDL_TEXTUREACCESS_TARGET,
                                            canvasSize.x * 2,
                                            canvasSize.y * 2);
        if(f) {
            currentLevel->windowResized(canvasSize, oldSize);
            currentLevel->mustRedraw();
            log.info("Resized window");
        }
        log.debug("Render size %dx%d", size.x, size.y);
        log.debug("Window size %dx%d", wsize.x, wsize.y);
        log.debug("Texture target size %dx%d", canvasSize.x * 2, canvasSize.y * 2);
        log.debug("Canvas size %dx%d", canvasSize.x, canvasSize.y);
        log.debug("Scale factor %f", scaleFactor);
    };
    resizeFunc(false);

    UILevel uiLevel(*this); uiLevel.ga.doubleIt = false;
    double fpslimit = 1.0/144.0;
    auto lastTimeGC = chrono::system_clock::now();
    timer.start();
    while(!this->quit) {
        pollEvents(fpslimit, resizeFunc);
        parseCommands();
        updateObjects(timer);

        SDL_Rect rekt = { 0, 0, canvasSize.x * 2, canvasSize.y * 2 };
        SDL_RenderSetViewport(this->renderer, &rekt);
        SDL_RenderSetScale(this->renderer, 1.0f, 1.0f);
        SDL_SetRenderTarget(this->renderer, rendererTexture);

        if(currentLevel->predraw()) {
            for(Object *obj : currentLevel->objects) {
                if(!obj->isInvisible()) obj->draw(currentLevel->ga);
            }
            currentLevel->draw();

            SDL_SetRenderTarget(this->renderer, nullptr);
            rekt = { 0, 0, size.x, size.y };
            SDL_RenderSetViewport(this->renderer, &rekt);
            SDL_RenderClear(this->renderer);
            SDL_RenderCopy(this->renderer, rendererTexture, nullptr, &rekt);
            SDL_RenderSetScale(this->renderer, scaleFactor, scaleFactor);

            //Draw UI Objects
            auto backup = currentLevel;
            currentLevel = &uiLevel;
            for(auto* &o: backup->uiObjects) {
                uiLevel.cameraPos = -o->frame.pos;
                uiLevel.focused = backup->focused;
                if(!o->isDisabled()) o->update(timer.getDelta(), uiLevel.ga);
                uiLevel.focused = backup->focused;
                if(!o->isInvisible()) o->draw(uiLevel.ga);
            }
            currentLevel = backup;

            SDL_RenderPresent(this->renderer);
        }

        deletePendingObjects();
        if(nextCurrentLevel) {
            currentLevel->cleanup();
            currentLevel = nextCurrentLevel;
            currentLevel->setup();
            log.debug("Changed to level %s", currentLevel->getName());
            nextCurrentLevel = nullptr;
        }

        timer.countFrame();
        if(timer.getDelta() < fpslimit) SDL_Delay(uint32_t((fpslimit - timer.getDelta()) * 1000));
        auto now = chrono::system_clock::now();
        chrono::duration<double> diff = now - lastTimeGC;
        if(diff.count() >= 1.0) {
            textCache_collect_garbage();
            lastTimeGC = now;
        }
    }

    SDL_DestroyTexture(rendererTexture);
    sendCommandResponse({ "", nullptr }, "");
}

void Game::end() {
    this->quit = true;
}

void Game::changeLevel(const char* name) {
    //this->currentLevel->cleanup();
    //this->currentLevel = &this->getLevel<Level>(name);
    //this->currentLevel->setup();
    //log.debug("Changed to level %s", name);
    nextCurrentLevel = &getLevel<Level>(name);
}

void Game::unsetPalette() {
    this->palette = nullptr;
}

void Game::importPalette(const string &path) {
    size_t len = path.length();
    if(!strcmp("aco", &path[len - 3])) {
        importPaletteFromPhotoshop(path);
    } else if(!strcmp("gpl", &path[len - 3])) {
        importPaletteFromGimp(path);
    } else {
        throw runtime_error("Unsupported palette");
    }
}

void Game::importPaletteFromGimp(const string &path) {
    if(path.rfind("gpl") == string::npos) {
        throw runtime_error("Palette file must end with .gpl extension");
    }

    InputFile i = openReadFile(path);
    if(!i.ok()) {
        throw runtime_error("Palette '" + path + "' not found or cannot be read");
    }

    setPalette(GimpPalette(i));
    i.close();
    log.debug("Imported GIMP palette %s", path.c_str());
}

void Game::importPaletteFromPhotoshop(const string &path) {
    if(path.rfind("aco") == string::npos) {
        throw runtime_error("Palette file must end with .aco extension");
    }

    InputFile i = openReadFile(path);
    if(!i.ok()) {
        throw runtime_error("Palette '" + path + "' not found or cannot be read");
    }

    setPalette(PhotoshopPalette(i));
    i.close();
    log.debug("Imported Photoshop palette %s", path.c_str());
}

const Palette& Game::getPalette() {
    return *this->palette;
}

void Game::loadFont(const string &str, size_t size) {
    this->font = TTF_OpenFont((this->gamePath + str).c_str(), int(size));
    if(this->font == nullptr) {
        throw runtime_error(string("Could not load ") + str + ": " + TTF_GetError());
    }
    log.debug("Loaded font %s", str.c_str());
    textCache_clear_all_entries();
}

void Game::captureMouse(bool capture) {
    SDL_SetRelativeMouseMode((SDL_bool) capture);
}

Game::Window Game::getWindow() {
    return { window, log, scaleFactor };
}

const Level* Game::getCurrentLevel() const {
    return currentLevel;
}

const Timer& Game::getTiming() const {
    return *timerPtr;
}

InputFile Game::openReadFile(const string &file, bool bin) const {
    return InputFile(gamePath + file, bin);
}

OutputFile Game::openWriteFile(const string &file, bool bin, bool app) const {
    return OutputFile(gamePath + file, bin, app);
}

InputOutputFile Game::openFile(const string &file, bool bin, bool app) const {
    return InputOutputFile(gamePath + file, bin, app);
}

void Game::saveGame(const char *saveName) {
    json saveJson, levels;
    saveJson["name"] = getWindow().getTitle();
    for(auto pairLevel: this->levels) {
        levels.push_back(*pairLevel.second);
    }
    saveJson["levels"] = levels;
    saveJson["currentLevel"] = currentLevel->getName();

    log.debug("Saving game status in '%s.save'", saveName);
    OutputFile outFile = openWriteFile(saveName + string(".save"), false);
    outFile.write(saveJson);
    outFile.close();
}

void Game::restoreGame(const char *saveName) {
    InputFile inFile = openReadFile(saveName + string(".save"), false);
    if(inFile.ok()) throw runtime_error("The save file '" + string(saveName) + "' doesn't exist");
    log.debug("Restoring game status from '%s.save'", saveName);
    json savedJson = json::parse(inFile.read());
    inFile.close();

    for(auto level: savedJson["levels"]) {
        string levelName = level["name"];
        this->levels[levelName]->restoreState(level);
    }

    string currentLevelName = savedJson["currentLevel"];
    currentLevel = this->levels[currentLevelName];
}

Game::~Game() {
    for(auto pair : this->levels) delete pair.second;
    textCache_clear_all_entries();

    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
    SDL_Quit();
}
