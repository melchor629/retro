#pragma once

#include <Object.hpp>
#include <MovableObject.hpp>
#include <functional>

namespace retro {

    class CollisionDetection: public Object {
    public:
        typedef std::function<void(MovableObject &, CollisionFace, const Frame &)> OnCollisionStart;
        typedef std::function<void(MovableObject &, CollisionFace, const Frame &)> OnCollisionEnd;

    private:
        MovableObject* object;
        OnCollisionStart start;
        OnCollisionEnd end;
        bool isColliding = false;
        CollisionFace lastFace;
        Frame lastCollision;

    protected:

        virtual void onCollisionStart(MovableObject &, CollisionFace, const Frame &) {}
        virtual void onCollisionEnd(MovableObject &, CollisionFace, const Frame &) {}

    public:
        CollisionDetection(Game &g, Level &l, const glm::vec2 &pos, const std::string &str, MovableObject* detectable): Object(g, l, pos, str) {
            assert(detectable != nullptr);
            object = detectable;
        }

        void setOnCollisionStartListener(const OnCollisionStart &listener) { start = listener; }
        void setOnCollisionStartListener(OnCollisionStart &&listener) { start = std::forward<OnCollisionStart>(listener); }
        void setOnCollisionEndListener(const OnCollisionEnd &listener) { end = listener; }
        void setOnCollisionEndListener(OnCollisionEnd &&listener) { end = std::forward<OnCollisionEnd>(listener); }

        constexpr void setFrame(const Frame &frame) { this->frame = frame; }
        constexpr void setFrame(const glm::vec2 &pos, const glm::vec2 &size) { this->frame = Frame{ pos, size }; }

        void setup() override {}
        void draw(GameActions &) override {}

        void update(float, GameActions &) override {
            CollisionFace face;
            Frame collision;
            std::tie(face, collision) = object->getFrame().collision(frame);
            if(face != CollisionFace::NONE && !isColliding) {
                isColliding = true;
                onCollisionStart(*object, face, collision);
                if(start) start(*object, face, collision);
            } else if(face != CollisionFace::NONE && isColliding) {
                lastFace = face;
                lastCollision = std::move(collision);
            } else if(face == CollisionFace::NONE && isColliding) {
                isColliding = false;
                onCollisionEnd(*object, lastFace, lastCollision);
                if(end) end(*object, lastFace, lastCollision);
            }
        }
        
    };

}
