#pragma once

#include <Player.hpp>
#include <GameActions.hpp>
#include <glm/geometric.hpp>

namespace retro {

    /// Player controlled by a human
    /**
     * Extends a Player to make it controllable by a human, using a
     * keyboard input. By default, WASD are the keys to move the player.
     * You can change it by changing {@link #upScancode}, {@link #downScancode},
     * {@link #leftScancode} and {@link #rightScancode}, creating a class
     * that extends ControlledPlayer. You can also modify the movement
     * behaviour by overriding the update() method. You must call movePlayer()
     * in when you want to make your player compute your movement with the
     * keyboard and Player::update() to make effective the movement.
     **/
    class ControlledPlayer: public Player {

    protected:

        int upScancode = 26; ///< Up
        int downScancode = 22; ///< Down
        int leftScancode = 4; ///< Left
        int rightScancode = 7; ///< Right

        float playerSpeed = 20.0f; ///< Player speed when moving

        ///< Calculates the new speed of the player based on the the keyboard input (or touch input)
        void movePlayer(GameActions &ga) {
            glm::vec2 dir = { 0, 0 };
            if(ga.isKeyPressed(upScancode) && !ga.isKeyPressed(downScancode)) dir.y = -1;
            if(ga.isKeyPressed(downScancode) && !ga.isKeyPressed(upScancode)) dir.y =  1;
            if(ga.isKeyPressed(leftScancode) && !ga.isKeyPressed(rightScancode)) dir.x = -1;
            if(ga.isKeyPressed(rightScancode) && !ga.isKeyPressed(leftScancode)) dir.x =  1;
#if defined(__ANDROID__) || defined(__IOS__)
            for(auto &touch: ga.getTouchPositions()) {
                if(touch.x < 0.3f and dir.x < 0.1f) dir.x = -1; else if(touch.x > 0.7f and dir.x > -0.1f) dir.x = 1;
                if(touch.y < 0.3f and dir.y < 0.1f) dir.y = -1; else if(touch.y > 0.7f and dir.y > -0.1f) dir.y = 1;
            }
#endif
            if(glm::length(dir) > 0.01f) {
                dir = glm::normalize(dir);
                if(abs(dir.x) > 0.01f) speed.x = playerSpeed * dir.x; else speed.x = 0.0f;
                if(abs(dir.y) > 0.01f) speed.y = playerSpeed * dir.y; else speed.y = 0.0f;
            }
        }

        ControlledPlayer(Game &g, Level &l, const glm::vec2 &pos, const std::string &name): Player(g, l, pos, name) {}

    public:

        virtual void update(float d, GameActions &ga) override {
            movePlayer(ga);
            Player::update(d, ga);
            speed *= 0.9f;
        }

        virtual void saveState(json &j) const override {
            Player::saveState(j);
            j["keys"]["up"] = upScancode;
            j["keys"]["down"] = downScancode;
            j["keys"]["left"] = leftScancode;
            j["keys"]["right"] = rightScancode;
            j["playerSpeed"] = playerSpeed;
        }

        virtual void restoreState(const json &j) override {
            Player::restoreState(j);
            upScancode = j["keys"]["up"];
            downScancode = j["keys"]["down"];
            leftScancode = j["keys"]["left"];
            rightScancode = j["keys"]["right"];
            playerSpeed = j["playerSpeed"];
        }

    };

}
