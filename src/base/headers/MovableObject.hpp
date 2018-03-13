#pragma once

#include <Object.hpp>

namespace retro {

    /// Represents an object that can be moved.
    /**
     * Objects that extends from this class can be moved automatically by only
     * setting the values {@link #speed} and {@link #acceleration}. The rest is
     * done after Level::update() is called.
     **/
    class MovableObject: public Object {

        glm::vec2 oldPos = { 0, 0 };
        float lastDelta = 1.0f / 60.0f;

    protected:

        glm::vec2 speed = { 0, 0 }; ///< Speed in canvas pixels/s
        glm::vec2 acceleration = { 0, 0 }; ///< Acceleration in canvas pixels/s^2
        glm::vec2 instantSpeed = { 0, 0 }; ///< Effective speed

        MovableObject(Game &game, Level &level, const glm::vec2 &pos, const std::string &name): Object(game, level, pos, name), oldPos(pos) {}

    public:

        /// Returns a preview of where the Movable Object will be after the update process
        Frame nextFrame(float delta) const {
            return { frame.pos + speed * delta + acceleration * delta * delta / 2.0f, frame.size };
        }

        virtual void update(float delta, GameActions&) override {
            instantSpeed = (frame.pos - oldPos) / (delta + lastDelta) * 2.0f;
            oldPos = frame.pos;
            lastDelta = delta;
            frame.pos += speed * delta + acceleration * delta * delta / 2.0f;
        }

        virtual void saveState(json &j) const override {
            Object::saveState(j);
            j["speed"] = speed;
            j["acceleration"] = acceleration;
        }

        virtual void restoreState(const json &j) override {
            Object::restoreState(j);
            speed = j["speed"];
            acceleration = j["acceleration"];
        }

        virtual ~MovableObject() {}

    };

}
