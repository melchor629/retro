#include "InventoryHUD.hpp"
#include "../objects/CollectableObject.hpp"
#include <GameActions.hpp>
#include <Game.hpp>
#include <Level.hpp>

using namespace hw;

InventoryHUD::InventoryHUD(Game &game, Level &level, const vec2 &pos, const string &name, const vector<CollectableObject*> &inventory): UIObject(game, level, pos, name), inventory(inventory) {

}

void InventoryHUD::draw(GameActions &ga) {
    //TODO Fill with code
}
