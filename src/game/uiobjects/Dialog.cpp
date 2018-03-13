#include "Dialog.hpp"
#include <GameActions.hpp>
#include <Game.hpp>
#include <Level.hpp>
#include <glm/vec2.hpp>
#include <utf8.h>

using namespace hw;
using namespace glm;

void Dialog::prepareAnimation() {
    //TODO Fill with code
}

void Dialog::setup() {
    //TODO Fill with code
}

void Dialog::update(float delta, GameActions &ga) {
    //TODO Fill with code
}

void Dialog::addPage(std::string &&str) {
    //TODO Fill with code
}

void Dialog::nextPage() {
    //TODO Fill with code
}

void Dialog::prevPage() {
    //TODO Fill with code
}

void Dialog::goToPage(size_t page) {
    //TODO Fill with code
}

bool Dialog::hasNextPage() {
    //TODO Fill with code
}

void Dialog::draw(GameActions &ga) {
    //TODO Fill with code
}

void Dialog::keyUp(int scancode) {
    UIObject::keyUp(scancode);
    if(scancode == SDL_SCANCODE_SPACE || scancode == SDL_SCANCODE_RETURN) {
        if(!aparecer.isCompleted()) {
            aparecer.complete();
        } else {
            if(hasNextPage()) {
                nextPage();
            } else {
                level.deleteObject(this);
            }
        }
    }
}

void Dialog::mouseUp(const ivec2 &pos, int button, int clicks) {
    UIObject::mouseUp(pos, button, clicks);
    if(button == SDL_BUTTON_LEFT) {
        if(!aparecer.isCompleted()) {
            aparecer.complete();
        } else {
            if(hasNextPage()) {
                nextPage();
            } else {
                level.deleteObject(this);
            }
        }
    }
}
