#pragma once

#include <glm/vec2.hpp>
#include <tuple>

namespace retro {

    /// Collision face
    enum CollisionFace {
        NONE = 0, LEFT = 1, RIGHT = 2, TOP = 4, BOTTOM = 8
    };

    constexpr CollisionFace operator|(const CollisionFace a, const CollisionFace b) {
        return static_cast<CollisionFace>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b));
    }
    
    constexpr CollisionFace operator&(const CollisionFace a, const CollisionFace b) {
        return static_cast<CollisionFace>(static_cast<unsigned char>(a) & static_cast<unsigned char>(b));
    }
    
    constexpr CollisionFace operator~(const CollisionFace a) {
        return static_cast<CollisionFace>(0xF & ~static_cast<unsigned char>(a));
    }

    /// Frame, also known as Rectangle
    /**
     * Represents a frame of something, the (rectangle) bounds of it. You can checc
     * if a point is inside it, or if another frame collides with this one. Also you
     * can simply know the position and the size of the frame.
     *
     * A frame is represented as a position (top-left), and from there, the size of
     * it. Negative width means that the position is represented on the right instead
     * of the left of the object. Negative height means that the position is represented
     * on the bottom instead of the top of the object. Both negative means bottom-right.
     **/
    struct Frame {
        glm::vec2 pos = { 0, 0 }; ///< Position of the Frame
        glm::vec2 size = { 0, 0 }; ///< Size of the frame

        inline bool collides(const Frame &o) const {
            float leftA = pos.x, rightA = pos.x + size.x;
            float topA = pos.y, bottomA = pos.y + size.y;
            float leftB = o.pos.x, rightB = o.pos.x + o.size.x;
            float topB = o.pos.y, bottomB = o.pos.y + o.size.y;

            return !(bottomA <= topB || topA >= bottomB || rightA <= leftB || leftA >= rightB);
        }

        /**
         * Calculates the collision and returns a Frame with the area that collides.
         * Only can return TOP|BOTTOM or LEFT|RIGHT.
         * @return tuple with the face where the collision is occurring and the frame of the collision
         **/
        std::tuple<CollisionFace, Frame> collision(const Frame &o) const {
            Frame diff;
            CollisionFace collision = CollisionFace::NONE;
            std::tie(collision, diff) = intersect(o);

            if(diff.size.x <= 0.0f || diff.size.y <= 0.0f) {
                diff.size.x = 0.0f;
                diff.size.y = 0.0f;
                collision = NONE;
            } else {
                if(diff.size.x < diff.size.y) {
                    collision = collision & ~(TOP | BOTTOM);
                    diff.size.y = 0;
                } else if(diff.size.y < diff.size.x) {
                    collision = collision & ~(LEFT | RIGHT);
                    diff.size.x = 0;
                }
            }
            
            return std::make_tuple(collision, diff);
        }

        /// Calculates the intersection and returns the intersect Frame and the faces that are colliding
        std::tuple<CollisionFace, Frame> intersect(const Frame &o) const {
            float leftA = pos.x, rightA = pos.x + size.x;
            float topA = pos.y, bottomA = pos.y + size.y;
            float leftB = o.pos.x, rightB = o.pos.x + o.size.x;
            float topB = o.pos.y, bottomB = o.pos.y + o.size.y;
            Frame diff;

            CollisionFace collision = CollisionFace::NONE;
            if(leftB > leftA) leftA = leftB;
            diff.pos.x = leftA;
            if(rightB < rightA) { rightA = rightB; collision = collision | RIGHT; } else { collision = collision | LEFT; }
            diff.size.x = rightA - diff.pos.x;
            if(topB > topA) topA = topB;
            diff.pos.y = topA;
            if(bottomB < bottomA) { bottomA = bottomB; collision = collision | BOTTOM; } else { collision = collision | TOP; }
            diff.size.y = bottomA - diff.pos.y;

            return std::make_tuple(collision, diff);
        }

        /// Check if a point is inside a Frame
        inline bool isInside(const glm::vec2 point) const {
            return pos.x <= point.x && point.x < pos.x + size.x && pos.y <= point.y && point.y < pos.y + size.y;
        }

        /// Returns the center of the frame
        inline glm::vec2 center() const {
            return { (pos.x + size.x / 2.0f), (pos.y + size.y / 2.0f) };
        }

        inline bool operator==(const Frame &other) const {
            return pos == other.pos && size == other.size;
        }

        inline bool operator!=(const Frame &other) const {
            return !(*this == other);
        }
    };

}
