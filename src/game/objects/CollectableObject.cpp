#include "CollectableObject.hpp"
#include <Game.hpp>
#include "../levels/FirstLevel.hpp"

using namespace hw;

void CollectableObject::onCollisionStart(MovableObject &obj, CollisionFace face, const Frame &frame) {//5
    levelAs<FirstLevel>().addToInventory(this);
    setInvisible(true);
    setDisabled(true);
    game().getAudio().playSample("Pickup");
}

void CollectableObject::onCollisionEnd(MovableObject &obj, CollisionFace face, const Frame &frame) {

}

void CollectableObject::draw(GameActions &ga) {
    sprite.draw_thicc({
        ivec2(round(frame.pos - vec2(desp) - ga.camera())) * 2,
        { 1, 1 }
    });
}

void CollectableObject::drawForUI(GameActions &ga) {
    sprite.draw({
        frame.pos - 8.0f * vec2(desp),
        { 8, 8 }
    });
}
