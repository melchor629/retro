#pragma once

#include <vector>
#include <ControlledPlayer.hpp>
#include <Sprites.hpp>
#include <Game.hpp>

namespace hw {

    using namespace std;
    using namespace retro;
    using namespace glm;

    class DaPlayer: public retro::ControlledPlayer {

        vector<Sprite> verticalAnimation;
        vector<Sprite> horizontalAnimation;
        vector<Sprite> diagonal1Animation;
        vector<Sprite> diagonal2Animation;
        int dir = 0;

        int i = 0;
        float timeAccum = 0.0f;

        inline vector<Sprite> &currentAnimation();

    public:

        DaPlayer(Game &game, Level &level, const glm::vec2 &pos, const char* name): ControlledPlayer(game, level, pos, name) {}

        void setup() override;

        void update(float d, GameActions &ga) override;

        void draw(GameActions& ga) override;

        void setSprites(const Sprites &sprites);

        void collisionWithMapSprite(const function<uint8_t(float, float)> &pixelAt, uint8_t prohibitedColor);

    };

}
