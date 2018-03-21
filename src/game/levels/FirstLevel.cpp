#include <Game.hpp>
#include "FirstLevel.hpp"
#include <MapObject.hpp>
#include <CollisionDetection.hpp>
#include "../objects/DaPlayer.hpp"
#include "../objects/CollectableObject.hpp"
#include "../uiobjects/Dialog.hpp"
#include "../uiobjects/InventoryHUD.hpp"

using namespace hw;

class SpriteObject: public Object, public Collisionable { //2
    Sprite sprite;
    vec2 desp;
public:
    SpriteObject(Game &g, Level &l, const vec2 &pos, const string &name, const Sprite &sprite, const vec2 &desp = {0,0}): Object(g, l, pos, name), sprite(sprite), desp(desp) {}

    virtual void setup() override {}
    virtual void update(float delta, GameActions &ga) override {}

    void draw(GameActions &ga) override {
        sprite.draw({ frame.pos - desp, {1,1} });
    }

    const Frame& getFrame() const override { return frame; }
};

constexpr vec2 dialogPos = { 100, 50 };

void FirstLevel::setup() {
    auto &map = addObject<MapObject>({ 0, 0 }, "first.map"); //1
    auto &player = addObject<DaPlayer>({ 10, 10 }, "player"); //1
    //auto &player = addObject<DaPlayer>({ 2, 98 }, "player");
    //auto &player = addObject<DaPlayer>({ 9*8+2, 16*8+2 }, "player");
    addObject<InventoryHUD>({ 0, 0 }, "inventoryHUD", inventory);
    auto &det1 = addObject<CollisionDetection>({ 0, 0 }, "detection1", getObjectByName<DaPlayer>("player"));//2
    auto &det2 = addObject<CollisionDetection>({ 0, 0 }, "detection2", getObjectByName<DaPlayer>("player"));//4
    auto &det3 = addObject<CollisionDetection>({ 0, 0 }, "detection3", getObjectByName<DaPlayer>("player"));//5
    auto &det4 = addObject<CollisionDetection>({ 0, 0 }, "detection4", getObjectByName<DaPlayer>("player"));//6
    auto &det5 = addObject<CollisionDetection>({ 0, 0 }, "detection5", getObjectByName<DaPlayer>("player"));//7
    auto &spr = *map.getSprites();//2
    auto &halfKey1 = addObject<CollectableObject>({ 0, 0 }, "halfKey1", &player, spr[17], uvec2(1, 0));//3
    auto &halfKey2 = addObject<CollectableObject>({ 0, 0 }, "halfKey2", &player, spr[18], uvec2(1, 0));//3
    auto &box = addObject<CollectableObject>({ 0, 0 }, "misteriousBox", &player, spr[16], uvec2(0, 0));//3
    addObject<SpriteObject>({ 16 * 8 + 1, 1 * 8 + 1 }, "collisionable1", spr[15], vec2(1, 1)).Object::getFrame().size = vec2 { 6, 6 };//2
    addObject<SpriteObject>({ 17 * 8 + 1, 2 * 8 + 1 }, "collisionable2", spr[15], vec2(1, 1)).Object::getFrame().size = vec2 { 6, 6 };//2

    player.setSprites(spr);//2
    det1.setFrame({ { 19*8, 2*8 + 4 }, { 8, 8 } });//2
    det1.setOnCollisionStartListener([this, &map] (auto &player, auto face, auto &col) {
        if(!playerHaveReceivedTheBeautifulIntroductionOfDoors && getObjectByName<Dialog>("dialog1") == nullptr) {
            addObject<Dialog>(dialogPos, "dialog1", initializer_list<string> {
                "You have found a door"s,
                "It will open for you only if instructed"s,
                "Buuuut, this game is programmed to open doors adjacent to you"s,
                "So doors will open automagically. Move on! Don't be a Schweinehund!"s
            });
            map.at(19, 2) = 29;
            map.regenerateTextures();
            player.setDisabled(true);
            playerHaveReceivedTheBeautifulIntroductionOfDoors = true;
            game().getAudio().playSample("Open Door");
        }
    });
    det1.setOnCollisionEndListener([this, &map] (auto &player, auto face, auto &col) {
        if(player.getFrame().pos.y >= 3*8) {
            map.at(19, 2) = 45;
            map.regenerateTextures();
            game().getAudio().playSample("Close Door");
        }
    });
    // ------- 4
    det2.setFrame({ { 7*8, 17*8 - 1 }, { 8, 9 } });
    det2.setOnCollisionStartListener([this, &map] (auto &player, auto face, auto &col) {
        if(player.getFrame().pos.y > 17*8) {
            map.at(7, 16) = 65;
            map.regenerateTextures();
            player.setDisabled(true);
            game().getAudio().playSample("Open Door");
        }
    });
    det2.setOnCollisionEndListener([this, &map] (auto &player, auto face, auto &col) {
        if(player.getFrame().pos.y <= 17*8) {
            map.at(7, 16) = 73;
            map.regenerateTextures();
            game().getAudio().playSample("Close Door");
        }
    });
    // ------- 5
    det3.setFrame({ { 8*8, 16*8 }, { 8, 8 } });
    det3.setOnCollisionStartListener([this, &map] (auto &player, auto face, auto &col) {
        if(!hasItemInInventory("halfKey1") || !hasItemInInventory("halfKey2")) {
            addObject<Dialog>(dialogPos, "dialog1", initializer_list<string> {
                "You have found another door"s,
                "[Player] Wait..."s,
                "[Player] This door is not opening."s,
                "[Player] There must be something I haven't found yet..."s
            });
            player.setDisabled(true);
        } else {
            map.at(8, 16) = 73;
            map.regenerateTextures();
            addObject<Dialog>(dialogPos, "dialog1", initializer_list<string> {
                "The two key pieces opens the door. You can continue your travel in this world."s,
                "[Player] Why I had to search for this broken key? The door has opened, although I couldn't fix the key."s,
                "[Player] And, where's the pieces?"s
            });
            player.setDisabled(true);
            removeFromInventory("halfKey1");
            removeFromInventory("halfKey2");
            deleteObject("detection3");
            game().getAudio().playSample("Open Door");
        }
    });
    // ------- 6
    det4.setFrame({ { 11*8 + 4, 15*8 }, { 1, 8*3 } });
    det4.setOnCollisionEndListener([this, &map] (auto &player, auto face, auto &col) {
        if(player.getFrame().pos.x > 11*8 + 4.5f) {
            map.at(11, 15) = 75;
            map.at(11, 16) = 75;
            map.at(11, 17) = 75;
            map.regenerateTextures();
            addObject<Dialog>(dialogPos, "dialog1", initializer_list<string> {
                "[Player] Oh fuck. I cannot go back…"s
            });
            player.setDisabled(true);
            deleteObject("detection4");
            game().getAudio().playSample("Close Door");
            game().getAudio().playSample("Close Door");
            game().getAudio().playSample("Close Door");
        }
    });
    // ------- 7
    det5.setFrame({ { 20*8 + 4, 16*8 }, { 4, 8 } });
    det5.setOnCollisionStartListener([this, &map] (auto &player, auto face, auto &col) {
        addObject<Dialog>(dialogPos, "dialog1", initializer_list<string> {
            "[Player] Ohhh c'mon… There's no exit!!"s,
            "[Player] What can I do???"s,
            "[Player] (thinking) mhhh..."s,
            "(still thinking) ..."s,
            "[Player] Hey you human, yes YOU HUMAN. Do something for me. Try to smash all keys"
            " of the keyboard, maybe pressing one key does something."s,
            "Maybe that's not a good idea, but do whatever you want…"s
        });
        player.setDisabled(true);
        deleteObject("detection5");
        stuckInThatShitAndHasToSmashTheKeyboard = true;
    });

    //5
    auto halfKeyCollisionListener = [this] (auto &player, auto face, auto &col) {
        if((!hasItemInInventory("halfKey1") && hasItemInInventory("halfKey2")) || (!hasItemInInventory("halfKey2") && hasItemInInventory("halfKey1"))) {
            addObject<Dialog>(dialogPos, "dialog1", initializer_list<string> {
                "You have found one pice of key"s,
                "Look for the other one"s
            });
            player.setDisabled(true);
        }
    };
    halfKey1.setFrame({ 4*8 + 2, 20*8 + 2 }, { 3, 4 });
    halfKey1.setOnCollisionStartListener(halfKeyCollisionListener);
    halfKey2.setFrame({ 8*8 + 2,  9*8 + 2 }, { 4, 4 });
    halfKey2.setOnCollisionStartListener(halfKeyCollisionListener);

    //5
    box.setFrame({ 0 + 3, 88 + 4 }, { 3, 2 });
    box.setOnCollisionStartListener([this] (auto &player, auto face, auto &col) {
        addObject<Dialog>(dialogPos, "dialog1", initializer_list<string> {
            "[Player] What the heck is this??"s,
            "You found a misterious box"s,
            "[Player] Really??? What is this?"s,
            u8"Maybe it is useful in the near future… Or maybe not…"s
        });
        player.setDisabled(true);
    });

    portals[{ 5, 4 }] = { 15, 0 }; //2
    portals[{ 12, 3 }] = { 0, 12 }; //3
    if(rand() % 2) {
        log.debug("Third section of portals, the good one is on the left");
        portals[{ 18, 12 }] = { 19, 17 }; //6
        portals[{ 20, 12 }] = { 20, 16 }; //6
    } else {
        log.debug("Third section of portals, the good one is on the right");
        portals[{ 18, 12 }] = { 20, 16 }; //6
        portals[{ 20, 12 }] = { 19, 17 }; //6
    }
    auto &audio = game().getAudio();
    audio.loadSample("aa.ogg");//1
    audio.loadSample("Open Door.wav");
    audio.loadSample("Close Door.wav");
    audio.loadSample("Pickup.wav");
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
    DaPlayer &player = *getObjectByName<DaPlayer>("player"); //1
    MapObject &map = *getObjectByName<MapObject>("first");
    const auto position = [&map, this] (const vec2 &pos) {
        return max(vec2{ 0, 0 }, min(pos - vec2(ga.canvasSize() / 2u), map.getFrame().size));
    };

    if(cameraAnimation.isCompleted()) {//2
        ga.camera(position(player.getFrame().pos - vec2(ivec2(player.getFrame().size) / 2)));//1 (primero esto, luego el if)
    } else if(!cameraAnimation.isCompleted()) {//2
        cameraAnimation.animate(delta);//2
    }

    uint8_t m = map.at((player.getFrame().pos + player.getFrame().size / 2.0f) / 8.0f); //2
    if(cameraAnimation.isCompleted() && (m == 14 || m == 30 || m == 46 || m == 62)) {
        auto it = portals.find(uvec2(player.getFrame().pos) / 8u);
        if(it != portals.end()) {
            cameraAnimation = Animation<vec2>(
                                              interpolator::CubicInOut<>(),
                                              2.0,
                                              player.getFrame().pos,
                                              it->second * 8u + uvec2(4, 4) - uvec2(player.getFrame().size) / 2u,
                                              [this, position, &player] (const vec2 &v) {
                                                  ga.camera(position(v));
                                                  player.getFrame().pos = v;
                                              });
            game().getAudio().playSample("aa", 0);
            player.setDisabled(true);
        }
    }

    if(m == 80) {
        if(fadeOutAnimation.isCompleted()) {
            if(fadeOutAlpha < 0.1f) {
                fadeOutAnimation = Animation<float>(interpolator::CubicOut<>(),
                                                    1.0,
                                                    0.0f,
                                                    255.0f,
                                                    [this] (const float &f) {
                                                        this->fadeOutAlpha = f;
                                                    });
                player.setDisabled(true);
            } else {
                game().changeLevel("endLevel");
            }
        } else {
            fadeOutAnimation.animate(delta);
        }
    }

    if(getObjectByName<Dialog>("dialog1") == nullptr && cameraAnimation.isCompleted()) { //2
        player.setDisabled(false);

        if(stuckInThatShitAndHasToSmashTheKeyboard && keysSmashed >= 41) { //7
            game().closeGame();
        }
    }
}

void FirstLevel::keyUp(int scancode) { //7
    Level::keyUp(scancode);
    if(stuckInThatShitAndHasToSmashTheKeyboard) {
        if(inventory.size() == 1) {
            if(scancode == SDL_SCANCODE_F) {
                getObjectByName<MapObject>("first")->at(20, 16) = 14;
                getObjectByName<MapObject>("first")->regenerateTextures();
                if(rand() % 2) {
                    portals[{ 20, 16 }] = { 19, 13 };
                } else {
                    portals[{ 20, 16 }] = { 19, 17 };
                }
                addObject<Dialog>(dialogPos, "dialog1", initializer_list<string> {
                    "[Player] Oooooooh, that box!"s,
                    "[Player] Oh my gosh, that misterious box saved me"s
                });
                getObjectByName<DaPlayer>("player")->setDisabled(true);
                removeFromInventory("misteriousBox");
            }
        } else {
            if(keysSmashed == 40) {
                addObject<Dialog>(dialogPos, "dialog1", initializer_list<string> {
                    "[Player] NOTHING WORKS!!! AAAAAAHHH!!!!"s,
                    "[Player] I think I will die here..."s
                });
                keysSmashed++;
            } else {
                keysSmashed++;
            }
        }
    }
}

bool FirstLevel::predraw() {//1
    ga.clear(000000_rgb);
    return true;
}

void FirstLevel::draw() {
    if(!fadeOutAnimation.isCompleted()) {
        ga.fillRectangle({ ga.camera(), ga.canvasSize() }, Color(0.f, 0.f, 0.f, round(fadeOutAlpha)));
    } else if(fadeOutAlpha > 0.9f) {
        ga.fillRectangle({ ga.camera(), ga.canvasSize() }, 0x000000_rgb);
    }
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
