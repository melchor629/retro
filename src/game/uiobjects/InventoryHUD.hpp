#pragma once

#include <UIObject.hpp>
#include <vector>

namespace hw {

    using namespace retro;
    using namespace std;
    using namespace glm;

    class CollectableObject;

    class InventoryHUD: public UIObject {

        const vector<CollectableObject*> &inventory;

    public:
        InventoryHUD(Game &game, Level &level, const vec2 &pos, const string &name, const vector<CollectableObject*> &inventory);

        void setup() override {};
        void draw(GameActions &ga) override;
    };

}
