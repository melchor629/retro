#pragma once

#include <string>
#include <glm/vec2.hpp>
#include <Sprites.hpp>
#include <json.hpp>

namespace retro {

    class GameActions;

    /// An object of the game.
    /**
     * The methods setup(), update() and draw() are called automatically. setup() when
     * the object is created, update() after Level::update() and draw() befor Level::draw().
     *
     * The object state can be stored when the game is saved by overriding saveState()
     * and restoreState(). **Don't forget** to call the super implementation.
     *
     * Objects that inherit from Object must have a constructor whose two first arguments
     * are `Game&`, `Level&`, `const glm::vec2&` and `const std::string name`. Otherwise,
     * you could not add the new object using Level::addObject(). If new parameters must
     * be added, add them at the end.
     *
     * An object can be disabled. When is disabled, the object will be drawn but won't
     * receive update calls. An object can be invisible, if it is, then won't be drawn.
     **/
    class Object {

        Game &g;

    protected:

        using json = nlohmann::json;

        Frame frame;
        std::string name;
        Level &level;
        bool disabled = false;
        bool invisible = false;
        Object(Game &game, Level &level, const glm::vec2 &pos, const std::string name): g(game), frame({ pos }), name(name), level(level) {}

        template<class GameType = Game>
        GameType& game() {
            return (GameType&) g;
        }

        template<class LevelType = Level>
        LevelType& levelAs() {
            return (LevelType&) level;
        }

    public:

        virtual void setup() = 0;
        virtual void update(float delta, GameActions &ga) = 0;
        virtual void draw(GameActions &ga) = 0;

        virtual Frame& getFrame() { return frame; }
        virtual const Frame& getFrame() const { return frame; }
        const char* getName() const { return name.c_str(); }

        constexpr bool isDisabled() { return disabled; }
        constexpr void setDisabled(bool dis) { disabled = dis; }

        constexpr bool isInvisible() { return invisible; }
        constexpr void setInvisible(bool inv) { invisible = inv; }

        virtual void saveState(json &j) const {
            j["name"] = getName();
            j["frame"] = getFrame();
        }

        virtual void restoreState(const json &j) {
            frame = j["frame"];
        }

        virtual ~Object() {}

    };

}
