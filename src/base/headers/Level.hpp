#pragma once

#include <Game.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <ControlledPlayer.hpp>
#include <GameActions.hpp>
#include <UIObject.hpp>
#include <algorithm>
#include "json.hpp"

namespace glm {
    using json = nlohmann::json;
    static void to_json(json &j, const vec2 &v);
    static void from_json(const json &j, vec2 &v);
}

namespace retro {

    class Map;
    class Sprites;
    struct Sprite;
    class Level;
    
    using json = nlohmann::json;
    void to_json(json &j, const Level &level);
    void from_json(const json &j, Level &level);
    static void to_json(json &j, const Color &c);
    static void from_json(const json &j, Color &c);

    /// A level of the game.
    /**
     * Holds the state, the logic and the draws of a level. Here is where the magicks happen.
     * To create a level, you must create a class that extends from this one and implement:
     *
     *  - Level() (the constructor, like in Game)
     *  - setup()
     *  - update()
     *  - draw()
     *  - **optional** predraw()
     *  - **optional** cleanup() (must call Level::cleanup() always to avoid memory leaks)
     *
     * Also, the Level provides some event listeners that you can easily catch up by overriding
     * a method in your implementation. These event listeners are:
     *
     *  - keyDown() When a key from the keyboard is pressed
     *  - keyUp() When a key from the keyboard is not pressed (but it was before)
     *  - keyText() When a key is pressed and corresponds to a character (useful for text input)
     *  - keyTextEdit() When some text edit action is happening
     *  - mouseDown() When a mouse button is pressed
     *  - mouseMoved() When a mouse button is not pressed (but it was before)
     *  - mouseWheelMoved() When the mouse wheel is moved
     *  - mustRedraw() When the window must be redrawn, told by the window compositor
     *
     * For the keyDown() and keyUp(), see [SDL_Scancode](https://wiki.libsdl.org/SDL_Scancode).
     *
     * Inside a Level, you can store anything you imagine (or need) and use it the way
     * you want. Only remember that you can make some actions through the GameObject
     * {@link #ga} object.
     *
     * The level can hold objects that have an automatic manage of the update and draw call.
     * When drawing, draws at the top the last object inserted, so you must ensure to put the
     * objects in the order you want to be rendered to avoid problems with layer ordering.
     * Check the type of the Object to know what actions are done automatically. See also
     * addObject(), getObject() and deleteObject().
     *
     * You can also save the state of the level so that the player can save the game, and
     * return to it recovering the game. To implement this, you don't need to do anything.
     * But, to store extra information about your level, you should override the following
     * methods (and don't forget to call the `super` implementation):
     *
     *  - saveState()
     *  - restoreState()
     *
     * If you need some debugging and/or logs, you have a Logger ready for be used in
     * {@link #log}.
     **/
    class Level {

        Game &g;
        std::vector<Object*> objects;
        std::vector<UIObject*> uiObjects;
        std::vector<Object*> pendingToDeleteObjects;
        UIObject* focused = nullptr;

    protected:

        friend class Game;
        friend class Map;
        friend class Sprites;
        friend struct Sprite;
        friend class GameActions;
        friend class UIObject;
        friend class Image;
        
        friend void to_json(json &j, const Level &level);
        friend void from_json(const json &j, Level &level);

        const char* name; ///< The name of the level.
        glm::vec2 cameraPos = { 0, 0 }; ///< The camera position. Use GameActions::camera() instead.
        Color lastColor = 0xFFFFFF_rgb; ///< Stores the fixed colour.
        GameActions ga; ///< GameAction
        Logger &log; ///< A Logger, to log things.
        Game::Audio &audio; ///< Audio object, to make audio things

        Level(Game &game, const char* name): g(game), name(name), ga(game, *this), log(Logger::getLogger(name)), audio(game.audio) {
            log.debug("Created level");
        }

        /// Returns the game object casted to the corresponding type (or to Game type by default)
        template<class GameType = Game>
        inline GameType& game() { return static_cast<GameType&>(this->g); }
        /// Gets the game path.
        constexpr const std::string& getGamePath() const { return g.gamePath; }
        /// Gets the palette.
        inline const Palette& getPalette() const { return g.getPalette(); }

        /// When a key from the keyboard is pressed.
        virtual void keyDown(int scancode) { if(focused != nullptr) focused->keyDown(scancode); }
        /// When a key from the keyboard is not pressed (but it was before).
        virtual void keyUp(int scancode) { if(focused != nullptr) focused->keyUp(scancode); }
        /// When a key is pressed and corresponds to a character (useful for text input).
        virtual void keyText(std::string ch) { if(focused != nullptr) focused->charKey(ch); }
        /// When some text edit action is happening.
        virtual void keyTextEdit(std::string ch, int start, int length) { if(focused != nullptr) focused->textEdit(ch, start, length); }
        /// When a mouse button is pressed.
        virtual void mouseDown(int button, int clicks) {
            for(auto* &ui: uiObjects) {
                if(ui->getFrame().isInside(ga.mpProfiteroles())) {
                    ui->mouseDown(ga.mpProfiteroles() - glm::ivec2(ui->getFrame().pos), button, clicks);
                }
            }
        }
        /// When a mouse button is not pressed (but it was before).
        virtual void mouseUp(int button, int clicks) {
            focused = nullptr;
            for(auto* &ui: uiObjects) {
                if(ui->getFrame().isInside(ga.mpProfiteroles())) {
                    if(ui->pressed & button) focused = ui;
                    ui->mouseUp(ga.mpProfiteroles() - glm::ivec2(ui->getFrame().pos), button, clicks);
                }
            }
        }
        /// When a mouse button is not pressed (but it was before).
        virtual void mouseMoved(const glm::ivec2 &pos, const glm::vec2 &desp) {
            for(auto* &ui: uiObjects) {
                if(ui->getFrame().isInside(ga.mpProfiteroles())) {
                    ui->mouseMoved(ga.mpProfiteroles() - glm::ivec2(ui->getFrame().pos), desp);
                } else if(ui->wasInside) {
                    ui->wasInside = false;
                    ui->mouseExit();
                }
            }
        }
        /// When the mouse wheel is moved.
        virtual void mouseWheelMoved(const glm::ivec2 &motion) {}
        /// When the window must be redrawn, told by the window compositor.
        virtual void mustRedraw() {}
        /// When the window has been resized
        virtual void windowResized(const glm::ivec2 &newSize, const glm::ivec2 &oldSize) {}

        /// Setup method. Called everytime a Level enters in the scene.
        virtual void setup() = 0;
        /// Update method, done before any object's update. If return `false`, no object will be updated.
        virtual bool preupdate(float delta) { return true; }
        /// Update method. Where (almost) all the logic occurs.
        virtual void update(float delta) = 0;
        /// If this method returns false, the method draw() is not called in this frame.
        /// You can use this method to draw things before {@link Object}s are drawn.
        virtual bool predraw() { return true; }
        /// Draw method. Where everything else (like the HUD) is be drawn.
        /// Draws over all {@link Object}s.
        virtual void draw() = 0;
        /// Cleanup method. Called when the Level won't be used anymore.
        virtual void cleanup() {
            for(Object* obj: objects) delete obj;
            for(UIObject* obj: uiObjects) delete obj;
        }
        
        /// Allows you to store your state in a object automatically
        virtual void saveState(json &object) const {
            object["cameraPos"] = cameraPos;
            object["lastColor"] = lastColor;
            object["name"] = getName();
            json ob;
            for(Object* const &o : objects) {
                json obj;
                o->saveState(obj);
                ob.push_back(obj);
            }
            object["objects"] = ob;
            json ui;
            for(UIObject* const &o : uiObjects) {
                json obj;
                o->saveState(obj);
                ui.push_back(obj);
            }
            object["uiObjects"] = ui;
        }
        
        /// Allows you to recover the state of the level
        virtual void restoreState(const json &object) {
            cameraPos = object["cameraPos"];
            lastColor = object["lastColor"];
            json ob = object["objects"];
            json ui = object["uiObjects"];
            for(size_t i = 0; i < ob.size(); i++) {
                json obj = ob.at(i);
                std::string objName = obj["name"];
                auto it = std::find_if(objects.begin(), objects.end(), [objName] (Object *o) { return !strcmp(o->getName(), objName.c_str()); });
                if(it == objects.end()) log.warn("The object %s stored in the state doesn't exist. Check your game!", objName.c_str());
                else (*it)->restoreState(obj);
            }
            for(size_t i = 0; i < ui.size(); i++) {
                json obj = ui.at(i);
                std::string objName = obj["name"];
                auto it = std::find_if(uiObjects.begin(), uiObjects.end(), [objName] (UIObject *o) { return !strcmp(o->getName(), objName.c_str()); });
                if(it == uiObjects.end()) log.warn("The UI object %s stored in the state doesn't exist. Check your game!", objName.c_str());
                else (*it)->restoreState(obj);
            }
        }

    public:

        constexpr GameActions& gameActions() { return ga; }

        /**
         * Adds an UIObject to the Level with a `name` and an initial `pos`ition.
         * After adding it, it will update its basic logic automatically and draw automatically.
         * Uses a parametrized type to instantiate the object. The UIObjects follows a different
         * render strategy: they're drawn after everything is drawn, and the canvas size is
         * different, allowing to make user friendly UI.
         * @param pos Initial position of the object
         * @param name Name of the object
         * @param args Extra arguments (if needed) for the object constructor
         * @return A reference to the new UI object
         **/
        template<
            class T,
            class ...Args,
            typename = std::enable_if_t<std::is_base_of<UIObject, T>{}>
        >
        T& addObject(const glm::vec2 &pos, const char* name, Args&&... args) {
            static_assert(!std::is_abstract<T>::value, "The type cannot be abstract. Implement all methods.");
            uiObjects.push_back(new T(game(), *this, pos, name, std::forward<Args>(args)...));
            uiObjects.back()->renderer = g.renderer;
            uiObjects.back()->gamePath = g.gamePath;
            uiObjects.back()->setup();
            log.debug("Added an UI object called %s", uiObjects.back()->getName());
            return (T&) *uiObjects.back();
        }

        /**
         * Adds an Object to the Level with a `name` and an initial `pos`ition.
         * After adding it, it will update its basic logic automatically and draw automatically.
         * Uses a parametrized type to instantiate the object. If the object extends from
         * UIObject, then the specialized method addObject() is called.
         * @param pos Initial position of the object
         * @param args **Name of the object** and extra arguments (if needed) for the object constructor
         * @return A reference to the new object
         **/
        template<class T, class ...Args, typename = std::enable_if_t<std::is_base_of<Object, T>::value && !std::is_base_of<UIObject, T>::value>>
        T& addObject(const glm::vec2 &pos, Args&&... args) {
            static_assert(!std::is_abstract<T>::value, "The type cannot be abstract. Implement all methods.");
            objects.push_back(new T(game(), *this, pos, std::forward<Args>(args)...));
            objects.back()->setup();
            log.debug("Added an item called %s", objects.back()->getName());
            return (T&) *objects.back();
        }

        /**
         * Adds a copy of an UIObject to the Level with a `name` and an initial `pos`ition.
         * After adding it, it will update its basic logic automatically and draw automatically.
         * The UIObjects follows a different render strategy: they're drawn after everything
         * is drawn, and the canvas size is different, allowing to make user friendly UI.
         * **Copies** the object passed.
         * @param obj An UIObject object to be copied (moved specifically).
         * @return A reference to the new ui object.
         **/
        template<
            class T,
            typename = std::enable_if_t<std::is_base_of<UIObject, T>{}>
        >
        T& addObject(T &&obj) {
            static_assert(!std::is_abstract<T>::value, "The type cannot be abstract. Implement all methods.");
            uiObjects.push_back(new T(std::forward<T>(obj)));
            uiObjects.back()->renderer = g.renderer;
            uiObjects.back()->gamePath = g.gamePath;
            uiObjects.back()->setup();
            log.debug("Added an UI object called %s", uiObjects.back()->getName());
            return (T&) *uiObjects.back();
        }

        /**
         * Adds a copy of an Object to the Level with a `name` and an initial `pos`ition.
         * After adding it, it will update its basic logic automatically and draw automatically.
         * **Copies** the object passed.
         * @param obj An Object item to be copied (moved specifically).
         * @return A reference to the new object.
         **/
        template<class T>
        T& addObject(const T &obj, typename std::enable_if<std::is_base_of<Object, T>::value && !std::is_base_of<UIObject, T>::value>::type* = 0) {
            static_assert(!std::is_abstract<T>::value, "The type cannot be abstract. Implement all methods.");
            objects.push_back(new T(obj));
            objects.back()->setup();
            log.debug("Added an item called %s", objects.back()->getName());
            return (T&) *objects.back();
        }

        /**
         * Finds an object by name. If cannot be found or the type doesn't match the one
         * requested (in its hierarchy of types), it will return `nullptr`. If more than
         * one object have the same name, it will return the first added.
         * @param name Name of the object
         * @return The object pointer or `nullptr`
         **/
        template<class T>
        T* getObjectByName(const char* name, typename std::enable_if<std::is_base_of<Object, T>::value && !std::is_base_of<UIObject, T>::value>::type* = 0) {
            auto it = std::find_if(objects.begin(), objects.end(), [name] (Object *o) { return !strcmp(o->getName(), name); });
            if(it != objects.end()) {
                if(dynamic_cast<T*>(*it) != nullptr) return (T*) *it;
            }
            return nullptr;
        }

        /**
         * Finds an object by position. If cannot be found or the type doesn't match the one
         * requested (in its hierarchy of types), it will return `nullptr`. If more than
         * one object is placed in that position, it will return the first added that matches
         * with the type requested (if there's an Object and a Player (in that order) and you
         * request a Player, it will return the Player).
         * @param pos Position where to find the object
         * @return The object pointer or `nullptr`
         **/
        template<class T>
        T* getObjectByPosition(const glm::vec2 &pos, typename std::enable_if<std::is_base_of<Object, T>::value && !std::is_base_of<UIObject, T>::value>::type* = 0) {
            auto it = objects.begin();
            while(it != objects.end()) {
                it = std::find_if(it, objects.end(), [&pos] (Object *o) { return o->getFrame().isInside(pos); });
                if(it != objects.end()) {
                    if(dynamic_cast<T*>(*it) != nullptr) return (T*) *it;
                    it++;
                }
            }
            return nullptr;
        }

        /**
         * Finds an object by index. If the index is invalid or the type doesn't match the one
         * requested (in its hierarchy of types), it will return `nullptr`.
         * @param i Index of the object
         * @return The object pointer or `nullptr`
         **/
        template<class T>
        T* getObjectByIndex(size_t i, typename std::enable_if<std::is_base_of<Object, T>::value && !std::is_base_of<UIObject, T>::value>::type* = 0) {
            if(i < objects.size()) {
                if(dynamic_cast<T*>(objects[i]) != nullptr) return (T*) objects[i];
            }
            return nullptr;
        }

        /**
          * Finds an UI object by name. If cannot be found or the type doesn't match the one
          * requested (in its hierarchy of types), it will return `nullptr`. If more than
          * one object have the same name, it will return the first added.
          * @param name Name of the UI object
          * @return The UI object pointer or `nullptr`
          **/
        template<class T, typename = std::enable_if_t<std::is_base_of<UIObject, T>::value>>
        T* getObjectByName(const char* name) {
            static_assert(std::is_base_of<UIObject, T>::value, "The type must be of type UIObject or derive from it.");
            auto it = std::find_if(uiObjects.begin(), uiObjects.end(), [name] (UIObject *o) { return !strcmp(o->getName(), name); });
            if(it != uiObjects.end()) {
                if(dynamic_cast<T*>(*it) != nullptr) return (T*) *it;
            }
            return nullptr;
        }

        /**
         * Finds an object and marks it to be deleted in the next tick (in the next frame).
         * It is done this way to avoid any kind of race conditions and problems with the
         * data structures used to hold the objects.
         **/
        void deleteObject(const char* name) {
            auto it = std::find_if(uiObjects.begin(), uiObjects.end(), [name] (UIObject *o) { return !strcmp(o->getName(), name); });
            if(it != uiObjects.end()) pendingToDeleteObjects.push_back(*it);
            else {
                auto ot = std::find_if(objects.begin(), objects.end(), [name] (Object *o) { return !strcmp(o->getName(), name); });
                if(ot != objects.end()) pendingToDeleteObjects.push_back(*ot);
            }
        }

        /**
         * Marks the object to be deleted in the next tick (in the next frame).
         * It is done this way to avoid any kind of race conditions and problems with the
         * data structures used to hold the objects.
         **/
        template<class T, typename = std::enable_if_t<std::is_base_of<UIObject, T>::value>>
        void deleteObject(T* object) {
            auto ot = std::find_if(uiObjects.begin(), uiObjects.end(), [object] (Object* o) { return o == object; });
            if(ot != uiObjects.end()) pendingToDeleteObjects.push_back(*ot);
        }

        /**
         * Marks the object to be deleted in the next tick (in the next frame).
         * It is done this way to avoid any kind of race conditions and problems with the
         * data structures used to hold the objects.
         **/
        template<class T>
        void deleteObject(T* object, typename std::enable_if<std::is_base_of<Object, T>::value && !std::is_base_of<UIObject, T>::value>::type* = 0) {
            auto ot = std::find_if(objects.begin(), objects.end(), [object] (Object* o) { return o == object; });
            if(ot != objects.end()) pendingToDeleteObjects.push_back(*ot);
        }
        
        /// Gets the name of the level.
        constexpr const char* getName() const { return name; }
        
        virtual ~Level() {}

    };
    
    static void to_json(json &j, const Color &c) {
        j["r"] = c.r;
        j["g"] = c.g;
        j["b"] = c.b;
        j["a"] = c.a;
    }
    
    static void from_json(const json &j, Color &c) {
        c = retro::rgba(j["r"], j["g"], j["b"], j["a"]);
    }

    static void to_json(json &j, const Frame &frame) {
        j["pos"] = frame.pos;
        j["size"] = frame.size;
    }

    static void from_json(const json &j, Frame &frame) {
        frame.pos = j["pos"];
        frame.size = j["size"];
    }

    template<typename T>
    static void to_json(json &j, const Optional<T> &opt) {
        if(opt) {
            j = *opt;
        } else {
            j = nullptr;
        }
    }

    template<typename T>
    static void from_json(const json &j, Optional<T> &opt) {
        if(j.is_null()) {
            opt = nullptr;
        } else {
            opt.value(j);
        }
    }
    
}

namespace glm {

    using json = nlohmann::json;
    static void to_json(json &j, const glm::vec2 &vec) {
        j["x"] = vec.x;
        j["y"] = vec.y;
    }

    static void from_json(const json &j, glm::vec2 &vec) {
        vec.x = j["x"];
        vec.y = j["y"];
    }

    static inline void to_json(json &j, const glm::ivec2 &vec) {
        j["x"] = vec.x;
        j["y"] = vec.y;
    }

    static inline void from_json(const json &j, glm::ivec2 &vec) {
        vec.x = j["x"];
        vec.y = j["y"];
    }

    static inline void to_json(json &j, const glm::uvec2 &vec) {
        j["x"] = vec.x;
        j["y"] = vec.y;
    }

    static inline void from_json(const json &j, glm::uvec2 &vec) {
        vec.x = j["x"];
        vec.y = j["y"];
    }
    
}
