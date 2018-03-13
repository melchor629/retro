#pragma once

#include <Frame.hpp>

namespace retro {

    /// Makes other objects collision with this one
    /**
     * Add this class to any object to make it collisionable with movable objects (aka MovableObject).
     * Due to a bad software engineering here, you must (re)implement these methods like this:
     * ```
     * virtual Frame& getFrame() { return frame; }
     * virtual const Frame& getFrame() const { return frame; }
     * ```
     * You must always do it, and the errors will disappear
     **/
    class Collisionable {

    public:

        virtual const Frame& getFrame() const = 0;

        virtual CollisionFace checkCollision(const Collisionable &c) {
            return std::get<0>(c.getFrame().collision(getFrame()));
        }

        virtual ~Collisionable() {}

    };

}
