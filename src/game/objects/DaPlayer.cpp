#include "DaPlayer.hpp"

using namespace hw;

vector<Sprite>& DaPlayer::currentAnimation() {
    if(abs(instantSpeed.x) < 0.001f && abs(instantSpeed.y) < 0.001f) {
        if(dir == 1) return verticalAnimation;
        if(dir == 2) return horizontalAnimation;
        if(dir == 3) return diagonal1Animation;
        if(dir == 4) return diagonal2Animation;
    } else {
        if(abs(instantSpeed.y) > 0.001f && abs(instantSpeed.x) <= 0.001f) { dir = 1; return verticalAnimation; }
        if(abs(instantSpeed.x) > 0.001f && abs(instantSpeed.y) <= 0.001f) { dir = 2; return horizontalAnimation; }
        if((instantSpeed.x > 0.0f && instantSpeed.y > 0.0f) || (instantSpeed.x < 0.0f && instantSpeed.y < 0.0f)) { dir = 3; return diagonal1Animation; }
        if((instantSpeed.x > 0.0f && instantSpeed.y < 0.0f) || (instantSpeed.x < 0.0f && instantSpeed.y > 0.0f)) { dir = 4; return diagonal2Animation; }
    }
    return verticalAnimation;
}

void DaPlayer::setup() {
    playerSpeed = 15.0;
}

void DaPlayer::update(float d, GameActions &ga) {
    ControlledPlayer::update(d, ga);
    if(abs(instantSpeed.x) < 0.09f && abs(instantSpeed.y) < 0.09f) {
        timeAccum = 0.0f;
        i = 0;
    } else {
        timeAccum += d;
        //First part is to make a 0.5 + [0, 1] values, then is multiplied by the delta and then
        //the speed you want to make the player for updating the sprites.
        float timeForSpeed = 2.0f * (0.75f - 0.5f * glm::length(instantSpeed) / playerSpeed) * d * 12.0f;
        //Avoid strange things when teleporting
        if(timeForSpeed > 0 && timeAccum >= timeForSpeed) {
            i = (i + 1) % currentAnimation().size();
            timeAccum = timeAccum - timeForSpeed;
        }
    }
}

void DaPlayer::draw(GameActions &ga) {
    Sprite spr = currentAnimation()[i];
    spr.draw(frame.pos - vec2{ 3, 3 });
}

void DaPlayer::setSprites(const Sprites &sprites) {

    frame.size = { 3, 3 };

    verticalAnimation.push_back(sprites[7]);
    verticalAnimation.push_back(sprites[8]);
    verticalAnimation.push_back(sprites[9]);
    verticalAnimation.push_back(sprites[8]);
    verticalAnimation.push_back(sprites[7]);
    verticalAnimation.push_back(sprites[6]);
    verticalAnimation.push_back(sprites[5]);
    verticalAnimation.push_back(sprites[6]);

    horizontalAnimation.push_back(sprites[23]);
    horizontalAnimation.push_back(sprites[24]);
    horizontalAnimation.push_back(sprites[25]);
    horizontalAnimation.push_back(sprites[24]);
    horizontalAnimation.push_back(sprites[23]);
    horizontalAnimation.push_back(sprites[22]);
    horizontalAnimation.push_back(sprites[21]);
    horizontalAnimation.push_back(sprites[22]);

    diagonal1Animation.push_back(sprites[39]);
    diagonal1Animation.push_back(sprites[40]);
    diagonal1Animation.push_back(sprites[41]);
    diagonal1Animation.push_back(sprites[40]);
    diagonal1Animation.push_back(sprites[39]);
    diagonal1Animation.push_back(sprites[38]);
    diagonal1Animation.push_back(sprites[37]);
    diagonal1Animation.push_back(sprites[38]);

    diagonal2Animation.push_back(sprites[55]);
    diagonal2Animation.push_back(sprites[56]);
    diagonal2Animation.push_back(sprites[57]);
    diagonal2Animation.push_back(sprites[56]);
    diagonal2Animation.push_back(sprites[55]);
    diagonal2Animation.push_back(sprites[54]);
    diagonal2Animation.push_back(sprites[53]);
    diagonal2Animation.push_back(sprites[54]);
}

void DaPlayer::collisionWithMapSprite(const function<uint8_t(float, float)> &pixelAt, uint8_t prohibitedColor) {
    auto frame = nextFrame(1/60);
    for(float x = 1; x < frame.size.x - 1; x += 1.0f) {
        while(pixelAt(frame.pos.x + x, frame.pos.y + cannotMoveDiff.y) == prohibitedColor) {
            cannotMoveUp = true;
            cannotMoveDiff.y += 1 - (frame.pos.y - std::floor(frame.pos.y));
        }
        while(pixelAt(frame.pos.x + x, frame.pos.y + frame.size.y + cannotMoveDiff.y) == prohibitedColor) {
            cannotMoveDown = true;
            auto diff = frame.pos.y - std::floor(frame.pos.y);
            if(abs(diff) < 0.001f) cannotMoveDiff.y -= 1;
            else cannotMoveDiff.y -= diff;
        }
    }
    for(float y = 1; y < frame.size.y - 1; y += 1.0f) {
        while(pixelAt(frame.pos.x + cannotMoveDiff.x, frame.pos.y + y + cannotMoveDiff.y) == prohibitedColor) {
            cannotMoveLeft = true;
            cannotMoveDiff.x += 1 - (frame.pos.x - std::floor(frame.pos.x));
        }
        while(pixelAt(frame.pos.x + frame.size.x + cannotMoveDiff.x, frame.pos.y + y + cannotMoveDiff.y) == prohibitedColor) {
            cannotMoveRight = true;
            auto diff = frame.pos.x - std::floor(frame.pos.x);
            if(abs(diff) < 0.001f) cannotMoveDiff.x -= 1;
            else cannotMoveDiff.x -= diff;
        }
    }
}
