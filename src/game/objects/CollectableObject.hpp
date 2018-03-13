#pragma once

#include <Object.hpp>
#include <CollisionDetection.hpp>
#include <Sprites.hpp>

namespace hw {

    using namespace std;
    using namespace retro;
    using namespace glm;

    class CollectableObject: public CollisionDetection {

        const Sprite sprite;
        const uvec2 desp;

        void onCollisionStart(MovableObject &, CollisionFace, const Frame &) override;
        void onCollisionEnd(MovableObject &, CollisionFace, const Frame &) override;

    public:

        CollectableObject(Game &game, Level &level, const vec2 &pos, const string &name, MovableObject* who, const Sprite &sprite, const uvec2 &desp):
            CollisionDetection(game, level, pos, name, who),
            sprite(sprite),
            desp(desp) {}

        void draw(GameActions &ga) override;
        void drawForUI(GameActions &ga);

    };

}
