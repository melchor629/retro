#include <Game.hpp>
#include "FirstLevel.hpp"
#include <MapObject.hpp>
#include <CollisionDetection.hpp>
#include "../objects/DaPlayer.hpp"
#include "../objects/CollectableObject.hpp"
#include "../uiobjects/Dialog.hpp"
#include "../uiobjects/InventoryHUD.hpp"

using namespace hw;

//TODO Fill with code

constexpr vec2 dialogPos = { 100, 50 };

void FirstLevel::setup() {
    //TODO Fill with code
}

bool FirstLevel::preupdate(float delta) {
    DaPlayer &player = *getObjectByName<DaPlayer>("player"); //2
    MapObject &map = *getObjectByName<MapObject>("first");
    const auto pixelAt = [&map] (float xx, float yy) {
        uint32_t x = floor(xx);
        uint32_t y = floor(yy);
        auto spriteNo = map.at(x / 8, y / 8);
        return map.getSprites()->at(spriteNo - 1).at(x % 8, y % 8);
    };
    auto prohibitedColor = 1;
    if(!player.isDisabled()) player.collisionWithMapSprite(pixelAt, prohibitedColor);
    return true;
}

void FirstLevel::update(float delta) {
    //TODO Fill with code
}

void FirstLevel::keyUp(int scancode) { //7
    //TODO Fill with code
}

bool FirstLevel::predraw() {//1
    //TODO Fill with code
}

void FirstLevel::draw() {
    //TODO Fill with code
}

void FirstLevel::addToInventory(CollectableObject *obj) { //5
    inventory.push_back(obj);
    log.info("Added '%s' to the inventory", obj->getName());
}

void FirstLevel::removeFromInventory(const string &name) {
    auto it = find_if(inventory.begin(), inventory.end(), [&name] (auto &item) { return name == item->getName(); });
    if(it != inventory.end()) {
        deleteObject(*it);
        inventory.erase(it);
        log.info("Deleted '%s' from the inventory", name.c_str());
    }
}

bool FirstLevel::hasItemInInventory(const string &name) const {
    auto it = find_if(inventory.begin(), inventory.end(), [&name] (auto &item) { return name == item->getName(); });
    return it != inventory.end();
}

void FirstLevel::clearInventory() {
    log.info("Clearing inventory...");
    for(auto &item: inventory) {
        deleteObject(item);
    }
    inventory.clear();
}
