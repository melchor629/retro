#include "InventoryHUD.hpp"
#include "../objects/CollectableObject.hpp"
#include <GameActions.hpp>
#include <Game.hpp>
#include <Level.hpp>

using namespace hw;

InventoryHUD::InventoryHUD(Game &game, Level &level, const vec2 &pos, const string &name, const vector<CollectableObject*> &inventory): UIObject(game, level, pos, name), inventory(inventory) {

}

void InventoryHUD::draw(GameActions &ga) {
    float left = 0.0f;
    for(auto item: inventory) {
        auto oldPos = item->getFrame().pos;
        auto frame = Frame{{ 10 + left, 10 }, item->getFrame().size * 16.0f + 2.0f * vec2(2, 6)};
        item->getFrame().pos = { 10 + left + 6, 10 + 6 };
        item->drawForUI(ga);
        item->getFrame().pos = oldPos;
        left += frame.size.x + 10;
    }
}
