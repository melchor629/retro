#include <Timer.hpp>

#if !defined(_WIN32) and !defined(__ANDROID__) and !defined(__IOS__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

using namespace retro;

void Timer::start() {
    this->started = true;
    this->paused = false;
    this->startTicks = SDL_GetTicks();
    this->lastFrameTicks = this->getTicks();
    this->pausedTicks = 0;
}

void Timer::stop() {
    this->started = false;
    this->paused = false;
    this->startTicks = 0;
    this->pausedTicks = 0;
}

void Timer::pause() {
    if(this->started && !this->paused) {
        this->paused = true;
        this->pausedTicks = SDL_GetTicks() - this->startTicks;
        this->startTicks = 0;
    }
}

void Timer::unpause() {
    if(this->started && this->paused) {
        this->paused = false;
        this->startTicks = SDL_GetTicks() - this->pausedTicks;
        this->pausedTicks = 0;
    }
}

bool Timer::isStarted() { return started; }
bool Timer::isPaused() { return paused; }

uint32_t Timer::getTicks() {
    uint32_t time = 0;
    if(started) {
        if(paused) {
            time = pausedTicks;
        } else {
            time = SDL_GetTicks() - startTicks;
        }
    }
    return time;
}

uint32_t Timer::getFrames() { return frames; }
double Timer::getDelta() { return delta; }

void Timer::countFrame() {
    auto readTicks = getTicks();
    delta = double(readTicks - lastFrameTicks) / 1000.0;
    frames++;
    lastFrameTicks = readTicks;
}
