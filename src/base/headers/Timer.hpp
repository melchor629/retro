#pragma once

#include <stdint.h>

namespace retro {

    /// Util class to calculate the FPS and the time between frames.
    class Timer {
        uint32_t startTicks = 0;
        uint32_t pausedTicks = 0;
        uint32_t frames = 0;
        uint32_t lastFrameTicks = 0;
        double delta = 1.0 / 60.0;
        bool paused = false;
        bool started = false;

    public:
        /// Starts the timer.
        void start();
        /// Stops the timer.
        void stop();
        /// Pauses the timer.
        void pause();
        /// Unpauses the timer.
        void unpause();

        /// Checks whether the Timer is started or not.
        bool isStarted();
        /// Checks whether the Timer is paused or not.
        bool isPaused();

        uint32_t getTicks();
        /// Returns the number of frames that were drawn in total.
        uint32_t getFrames();
        /// Gets the last time elapsed between the two frames. To get FPS use \f$1/getDelta()\f$.
        double getDelta();
        /// Takes note that one frame has occured, and calculates everything.
        void countFrame();
    };

}
