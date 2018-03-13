#pragma once

#include <Level.hpp>
#include <Animation.hpp>
#include <unordered_map>
#include <vector>

template<> struct std::hash<glm::uvec2> {
    size_t operator()(const glm::uvec2 &v) const {
        auto hs = std::hash<uint32_t>{};
        return hs(v.r) ^ (hs(v.g) << 16);
    }
};

namespace hw {
    class CollectableObject;
}

namespace hw {

    using namespace std;
    using namespace retro;
    using namespace glm;

    class FirstLevel: public Level {
        Animation<vec2> cameraAnimation;
        Animation<float> fadeOutAnimation;
        unordered_map<uvec2, uvec2> portals;
        vector<CollectableObject*> inventory;
        bool stuckInThatShitAndHasToSmashTheKeyboard = false;
        bool playerHaveReceivedTheBeautifulIntroductionOfDoors = false;
        int keysSmashed = 0;
        float fadeOutAlpha = 0.0f;
    protected:
        void setup() override;

        void keyUp(int scancode) override;

        bool preupdate(float delta) override;

        void update(float delta) override;

        bool predraw() override;

        void draw() override;

    public:
        FirstLevel(Game &game, const char* name): Level(game, name) {}

        void addToInventory(CollectableObject* obj);
        void removeFromInventory(const string &name);
        void clearInventory();
        bool hasItemInInventory(const string &name) const;
        const vector<CollectableObject*>& getInventory() const { return inventory; };
    };

}
