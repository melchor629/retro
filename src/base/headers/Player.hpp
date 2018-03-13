#pragma once

#include <MovableObject.hpp>
#include <Collisionable.hpp>

namespace retro {

    /// A Collisionable MovableObject that represents an *intelligent* entity.
    /**
     * This kind of entities collides with other collisionable objects and with the maps.
     * Furthermore, they are moved by some unknown-omnipowerful-entity.
     *
     * Implementations of Player are suitable for NPCs. Use in conjunction with a Sprite
     * to make a more beautiful Player.
     **/
    class Player: public MovableObject, public Collisionable {

    protected:

        bool cannotMoveUp = false;
        bool cannotMoveDown = false;
        bool cannotMoveLeft = false;
        bool cannotMoveRight = false;
        glm::vec2 cannotMoveDiff = { 0, 0 };

        Player(Game &game, Level &level, const glm::vec2 &pos, const std::string &name): MovableObject(game, level, pos, name) {}

    public:

        /// Checks whether this Player collides with a Collisionable, and if it is, it will move the Player back to a non-colliding position.
        CollisionFace checkCollision(const Collisionable &c) override {
            CollisionFace dir;
            Frame diff;
            std::tie(dir, diff) = c.getFrame().collision(getFrame());

            if(dir != NONE) {
                if(dir & BOTTOM) { cannotMoveDown = true; cannotMoveDiff.y -= diff.size.y; }
                else if(dir & TOP) { cannotMoveUp = true; cannotMoveDiff.y += diff.size.y; }
                if(dir & LEFT) { cannotMoveLeft = true; cannotMoveDiff.x += diff.size.x; }
                else if(dir & RIGHT) { cannotMoveRight = true; cannotMoveDiff.x -= diff.size.x; }
                speed = { 0, 0 };
                acceleration = { 0, 0 };
            }

            return dir;
        }

        /// Checks whether this Player collides with the map, and if it is, it will move the Player back to a non-colliding position. It won't check for the map bounds.
        virtual void collisionWithMap(CollisionFace c) {
            if(c & TOP) {
                cannotMoveUp = true;
                auto mod = fmod(frame.pos.y, 8);
                if(abs(mod) > 0.001f && mod > frame.size.y / 2) {
                    cannotMoveDiff.y += 8.0 - mod;
                }
            }
            if(c & BOTTOM) {
                cannotMoveDown = true;
                auto mod = fmod(frame.pos.y + frame.size.y, 8);
                if(abs(mod) > 0.001f && mod < frame.size.y / 2) {
                    cannotMoveDiff.y -= mod;
                }
            }
            if(c & LEFT) {
                cannotMoveLeft = true;
                auto mod = fmod(frame.pos.x, 8);
                if(abs(mod) > 0.001f && mod > frame.size.x / 2) {
                    cannotMoveDiff.x += 8.0 - mod;
                }
            }
            if(c & RIGHT) {
                cannotMoveRight = true;
                auto mod = fmod(frame.pos.x + frame.size.x, 8);
                if(abs(mod) > 0.001f && mod < frame.size.y / 2) {
                    cannotMoveDiff.x -= mod;
                }
            }
        }

        void update(float f, GameActions &ga) override {
            if(cannotMoveUp) {
                cannotMoveUp = false;
                if(speed.y >= 0 && acceleration.y >= 0) frame.pos.y += cannotMoveDiff.y;
                if(speed.y < 0) speed.y = 0;
                if(acceleration.y < 0) acceleration.y = 0;
            }
            if(cannotMoveDown) {
                cannotMoveDown = false;
                if(speed.y <= 0 && acceleration.y <= 0) frame.pos.y += cannotMoveDiff.y;
                if(speed.y > 0) speed.y = 0;
                if(acceleration.y > 0) acceleration.y = 0;
            }
            if(cannotMoveLeft) {
                cannotMoveLeft = false;
                if(speed.x >= 0 && acceleration.x >= 0) frame.pos.x += cannotMoveDiff.x;
                if(speed.x < 0) speed.x = 0;
                if(acceleration.x < 0) acceleration.x = 0;
            }
            if(cannotMoveRight) {
                cannotMoveRight = false;
                if(speed.x <= 0 && acceleration.x <= 0) frame.pos.x += cannotMoveDiff.x;
                if(speed.x > 0) speed.x = 0;
                if(acceleration.x > 0) acceleration.x = 0;
            }
            cannotMoveDiff = { 0, 0 };
            MovableObject::update(f, ga);
        }

        Frame& getFrame() override { return frame; }
        const Frame& getFrame() const override { return frame; }

        virtual ~Player() {}

    };

}
