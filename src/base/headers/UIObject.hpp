#pragma once

#include <Object.hpp>
#include <Color.hpp>
#include <Optional.hpp>

typedef struct _TTF_Font TTF_Font;
typedef struct SDL_Renderer SDL_Renderer;
struct CacheValue;

namespace retro {

    /// Determines the font style to be rendered.
    /**
     * Enum type that can be joined (using `&` operator) to form complex
     * styles like Bold and Italic (`FontStyle::Bold & FontStyle::Italic`).
     *
     * **Note** that some fonts only implement one type of font style, so
     * if the effect is not working, maybe is due to that.
     */
    enum class FontStyle: uint8_t {
        Normal = 0,         ///< No effects to the font
        Bold = 1,           ///< **Bold** effect
        Italic = 2,         ///< *Italic* effect
        Underline = 4,      ///< Underline effect
        Strikethrough = 8   ///< Strikethrough effect
    };

    /// Determines the limits of the text box/frame
    /**
     * The text frame is determined by:
     *  - FixedWidthAndHeight: you decide what is the width and height of the box
     * and is 100% up to you.
     *  - FixedWidth: You only decide what is the width, and the height is calculed
     * by the maximum of the current height and the text height.
     *  - Nothing: The size is calculed with the maximum of the before value and
     * with the text width and height.
     **/
    enum class BoxLimit: uint8_t {
        FixedWidthAndHeight = 0, ///< 100% manual
        FixedWidth = 1,          ///< 50% manual
        Nothing = 2              ///< Fight!
    };

    /// How the text must be aligned horizontaly
    enum class TextHorizontalAlign: uint8_t {
        Left, Center, Right
    };

    /// How the text must be aligned verticaly
    enum class TextVerticalAlign: uint8_t {
        Top, Center, Bottom
    };

    /// A different kind of Object, UIObject is for the interface
    /**
     * UIObjects are the special kind of Object that are suitable to be
     * used in the HUD (Game Interface). They are rendered using the full
     * window resolution, using an isolated environment that cannot affect
     * some state of the drawing context used in normal rendering.
     *
     * The UIObject is rendered after everything else, but only if Level::predraw()
     * returns `true`. Also, this objects are updated just before they are drawn,
     * and are stored in different data structures.
     *
     * The UIObject allows you to use the useful GameAction methods (except
     * for GameAction::print()) and use different fonts (different from the one in
     * Game::loadFont()) and custom text styling. Only one text can be rendered per
     * UIObject, but you can decide where the text is must be shown inside the object.
     * See setFont(), setFontStyle(), setFontOutline(), setText(), setTextColor() (or
     * setTextColour()), setAlign(), setTextBoxLimit() and renderText(). All these
     * methods are protected, so you must call them in the Object::setup(),
     * Object::draw() or Object::update().
     *
     * The Object::frame attribute is semicalculated by the UIObject. Let me explain.
     * The position is up to you 100%, but the size is controlled by you but with some
     * restrictions. The size of the frame must be large enought to hold the whole text
     * in the position you put it. If the frame has a small size, the UIObject will
     * resize it. If you move the text and now there's a gap in the frame, the
     * UIObject won't resize the frame. You can always check for the size of the text
     * using getTextSize(), that is independent size from Object::frame.
     *
     * The UIObject stores in cache the rendered version of the text. Any of the
     * following method calls will clear the cache, but only if there's a difference
     * between the current value and the new: setTextBoxLimit() (_will clear it always_),
     * setFont(), setFontStyle(), setText(), setTextColor(), restoreState().
     *
     * Well, there's a lot about text. But, _what you should do if you don't want to
     * render text?_ Simply, don't call any text/font related method :) But if you want,
     * it is recommended to prepare the font and the text in a Object::setup() method.
     *
     * To put different texts (or UIObjects) in the same UIObject, I recommend you
     * to add them using addSubobject(). This method allows you to use relative
     * positions to the same UIObject, making you to not worry about the position
     * of them in the screen. Imagine that you have a UIObject in (10, 10), and
     * add to it a new one at (20, 10). That means that the second object will
     * render at (30, 20) because uses a relative positioning from the parent.
     * An example of adding a sub-object:
     *
     *     UILabel label(this, { 10, 10 }, "label2");
     *     label.setFont("GenericFont.ttf", 24);
     *     label.setText("A label");
     *     addSubobject(std::move(label));
     *
     * The `std::move` is important, to remark that the label is _moving_ to the
     * new container.
     *
     * The subobjects can modify the frame size too, in the same way it does the
     * text when BoxLimit::Nothing is set. Must ensure that the objects fit inside
     * the parent. No clip can be done.
     *
     * For the input events, there's a bunch of methods that can be overridden to
     * know when an input event has occurred. **Always call the super-implementation**,
     * to avoid strange behaviours. The methods you can use are: focus(), lostFocus(),
     * keyDown(), keyUp(), charKey(), mouseEnter(), mouseExit(), mouseDown(),
     * mouseUp(), mouseClick() and mouseMoved(). Some of these events are called even
     * if the action goes to a subobject. See their documentation to known when are
     * called.
     *
     *  > **Note**: All the get and set functions (and the renderText() too) supposes
     *  > that a font is loaded (except for loadFont() obviusly). If a font is not loaded
     *  > and one of these methods are called, the behaviour is undefined and even your
     *  > game could crash.
     **/
    class UIObject: public Object {

        SDL_Renderer* renderer;
        TTF_Font* font = nullptr;
        CacheValue* cacheValue = nullptr;
        glm::uvec2 textFrame = { 0, 0 };
        BoxLimit boxLimit = BoxLimit::Nothing; /// < Use setTextBoxStyle() instead of this
        std::string text;
        Color color = 0xFFFFFF_rgb;
        std::string gamePath;
        std::string fontPath;
        uint32_t fontSize = 0;
        TextHorizontalAlign hAlign = TextHorizontalAlign::Left;
        TextVerticalAlign vAlign = TextVerticalAlign::Top;
        std::vector<UIObject*> subObjects;
        UIObject* parent = nullptr, *focused = nullptr;
        bool wasInside = false, isFocused = false;
        int pressed = 0;

        /// Does some magics for getAlign()
        struct TextAlign {
            const TextVerticalAlign a;
            const TextHorizontalAlign b;
            constexpr operator TextVerticalAlign() { return a; }
            constexpr operator TextHorizontalAlign() { return b; }
        };

        void clearCache();
        void generateCache();
        void setFontWithPath(const std::string &path, uint32_t size);

    protected:

        friend Game;
        friend Level;

        UIObject(Game &game, Level &level, const glm::vec2 &pos, const std::string &name);
        UIObject(UIObject* parent, const glm::vec2 &pos, const std::string &name);

        /**
         * Renders the text, at last! The positions is relative to the Object::frame
         * position. Can modify the Object::frame size to make the text fit inside.
         * Call this method in a UIObject::draw() method. And call it once.
         * @param ga A GameActions reference
         * @param pos The position of the text, (0, 0) by default
         **/
        void renderText(GameActions &ga, const glm::vec2 &pos = { 0, 0 });

        /// Event notified when the UIObject gains the input focus
        virtual void focus() {}
        /// Event notified when the UIObject losses the input focus
        virtual void lostFocus() {}
        /// When a key is pressed, the event is spread among all the UIObjects
        /**
         * The method is called when a key is pressed and either the UIObject
         * has the input focus or one of its subobjects has it. If you need to
         * do something, you should check first if the UIObject (your UIObject)
         * has the focus with hasInputFocus().
         *
         * The `key` argument is a scancode key obtained directly from one of
         * the `SDL_SCANCODE_*` enumeration values. The full list can be seen
         * [here](https://wiki.libsdl.org/SDL_Scancode).
         * @param key The `SDL_SCANCODE_*` key
         */
        virtual void keyDown(int key) { if(focused) focused->keyDown(key); }
        /// When a key stops being pressed, the event is spread among all the UIObjects
        /**
         * The method is called when a key was pressed and now not, and either
         * the UIObject has the input focus or one of its subobjects has it.
         * If you need to do something, you should check first if the UIObject
         * (your UIObject) has the focus with hasInputFocus().
         *
         * The `key` argument is a scancode key obtained directly from one of
         * the `SDL_SCANCODE_*` enumeration values. The full list can be seen
         * [here](https://wiki.libsdl.org/SDL_Scancode).
         * @param key The `SDL_SCANCODE_*` key
         */
        virtual void keyUp(int key) { if(focused) focused->keyDown(key); }
        /// When a key is pressed, and can be translated into an UTF-8 character
        /**
         * The method is called when a key was pressed and the key can be translated
         * into an UTF-8 character. The C string is that character codified as UTF-8.
         * For correct text input in UIObjects, GameActions::startInputText() should
         * be called. When the event is fired, you shall add to the end of the text
         * (append) the string received.
         *
         * Also this method is called either if the UIObject has the input focus
         * or one of its subobjects has it. If you need to do something, you should
         * check first if the UIObject (your UIObject) has the focus with hasInputFocus().
         *
         * To make a delete character, you will need to listen for `SDL_SCANCODE_BACKSPACE`
         * in keyUp() in case the object has the focus and can delete a character. As
         * the text is UTF-8 encoded, sometimes delete the last characer doesn't mean
         * delete the last element of the string. You will need [utf8::prior()](https://github.com/nemtrif/utfcpp#utf8prior)
         * for that.
         *
         * ```
         * void keyUp(int key) override {
         *     UIObject::keyUp(key); //Always call super implementation!
         *     if(hasFocus() && key == SDL_SCANCODE_BACKSPACE && !text.empty()) {
         *         auto prev = text();
         *         utf8::prior(prev, text()); //Searches for the last Unicode code point in the UTF-8 string
         *         text = text.substr(0, prev - text()); //Delete it
         *     }
         * }
         * void charKey(std::string input) override {
         *     UIObject::charKey(input); //Remember to do that always :)
         *     text += input;
         * }
         * ```
         *
         * @param input The new input character codified as a C UTF-8 string
         */
        virtual void charKey(std::string input) { if(focused) focused->charKey(input); }
        /// When a key is pressed, the input is compositing a new character (like `´+a` -> `á`)
        /**
         * The method is called when a key was pressed and the key can be translated
         * into an UTF-8 character, but is not a end character. Involves a composition
         * of characters that ends into a final character. Like when someone want to
         * write the character `á`, first will write the accent `´` and then will press
         * the `a` to form the new character. In this process, the engine will call two
         * methods:
         *
         *  - `textEdit("´", 1, 0);` Starts compositing a new complex character
         *  - `charKey("á");` Character is composed, it is safe to append it
         *
         * Allows users that have complex written languages (like Chinese, Japanese) or Latin
         * languages to write complex characters and have an useful feedback (that must be
         * implemented by you) showing what are writing. You should keep these characters in
         * a different `string` and draw them at the end of the text. Remember to clear the
         * second string when charKey() is called.
         *
         * The C string is a partial text codified into UTF-8. The input text mode shall call
         * GameActions::startInputText() and (when done) call GameActions::endInputText().
         * Notice that this method is called either if the UIObject has the input focus
         * or one of its subobjects has it. If you need to do something, you should
         * check first if the UIObject (your UIObject) has the focus with hasInputFocus().
         * @param text The composition characters codified as a C UTF-8 string
         * @param start The location to begin editing from
         * @param len The number of characters to edit from the start point
         */
        virtual void textEdit(std::string text, int start, int len) { if(focused) focused->textEdit(text, start, len); }
        virtual void mouseEnter() {}
        virtual void mouseExit() { pressed = 0; }
        virtual void mouseDown(const glm::ivec2 &pos, int button, int clicks);
        virtual void mouseUp(const glm::ivec2 &pos, int button, int clicks);
        virtual void mouseClick(int) {}
        virtual void mouseMoved(const glm::ivec2 &pos, const glm::ivec2 &desp);

    public:

        UIObject(const UIObject &o): Object(o) {
            renderer = o.renderer;
            font = o.font;
            cacheValue = o.cacheValue;
            textFrame = o.textFrame;
            boxLimit = o.boxLimit;
            text = o.text;
            color = o.color;
            gamePath = o.gamePath;
            fontPath = o.fontPath;
            fontSize = o.fontSize;
            hAlign = o.hAlign;
            vAlign = o.vAlign;
            subObjects = o.subObjects;
            parent = o.parent;
            focused = o.focused;
            wasInside = o.wasInside;
            isFocused = o.isFocused;
            pressed = o.pressed;
        }

        /// Move constructor
        UIObject(UIObject &&o): Object(std::forward<UIObject>(o)) {
            constexpr auto moveAndNull = [] (auto* &a) -> auto* { auto* o = a; a = nullptr; return o; };
            renderer = o.renderer;
            font = moveAndNull(o.font);
            cacheValue = moveAndNull(o.cacheValue);
            textFrame = std::move(o.textFrame);
            boxLimit = o.boxLimit;
            text = std::move(o.text);
            color = std::move(o.color);
            gamePath = o.gamePath;
            fontPath = std::move(o.fontPath);
            fontSize = o.fontSize;
            hAlign = o.hAlign;
            vAlign = o.vAlign;
            subObjects = std::move(o.subObjects);
            parent = moveAndNull(o.parent);
            focused = moveAndNull(o.focused);
            wasInside = o.wasInside;
            isFocused = o.isFocused;
            pressed = o.pressed;
        }

        /// Returns `true` if this UIObject has the input focus
        constexpr bool hasInputFocus() const { return isFocused && focused == nullptr; }

        /**
         * Changes the box limit type with the one specified in BoxLimit.
         * Optionally, you can define a new text box/frame size, or let
         * the mode select one for you. Clears the cache always.
         * @param style The BoxLimit _(style, I don't know why the name style)_
         * @param s The new size, optionally
         **/
        void setTextBoxLimit(BoxLimit style, const glm::ivec2 s = { 0, 0 });

        /// Gets the text BoxLimit mode
        BoxLimit getTextBoxLimit();

        /**
         * Sets the font to the `ttf` file inside the game path and a size in
         * px (base on 72PPP). Can clear the cache.
         * @param name Path to the font
         * @param size Size in pixels
         */
        void setFont(const std::string &name, uint32_t size);

        /// Gets the Font Family name
        std::string getFontName();

        /// Gets the font size
        uint32_t getFontSize();

        /**
         * Changes the FontStyle. Notice that in some fonts, only one
         * font style is represented, so sometimes the style is not applied
         * or another style is being applied. Can clear the cache.
         * @param style The FontStyle to apply
         **/
        void setFontStyle(FontStyle style);

        /// Gets the FontStyle applied
        FontStyle getFontStyle();

        /**
         * Changes the font outline. The outline is that kind of border around
         * the text. Can clear the cache.
         * @param outlinepx The outline in pixels
         */
        void setFontOutline(uint32_t outlinepx);

        /// Gets the font outline in pixels
        uint32_t getFontOutline();

        /**
         * Changes the text to be rendered to the new one. New lines can be
         * represented too. Can clear the cache.
         * @param str The new text
         */
        void setText(const std::string &str);

        /// Gets the text (reference) of the UIObject
        constexpr std::string& getText() { return text; }

        /**
         * Changes the text color. Can clear the cache.
         * @param c The new color
         */
        void setTextColor(Color c);

        /// British version of setTextColor()
        void setTextColour(Colour c) { setTextColor(c); }

        /// Gets the color of the text
        constexpr Color getTextColor() const { return color; }

        /// Gets the British version of the colour of the text
        constexpr Colour getTextColour() const { return color; }

        /// Changes the vertical align
        constexpr void setAlign(TextHorizontalAlign h) { hAlign = h; }

        ///Changes the horizontal align
        constexpr void setAlign(TextVerticalAlign v) { vAlign = v; }

        ///Returns the TextVerticalAlign or TextHorizontalAlign, dependening on your needs ;)
        constexpr TextAlign getAlign() { return { vAlign, hAlign }; }

        /// Gets the text size (the size is calculated after the first draw)
        constexpr glm::ivec2 getTextSize() const { return textFrame; }

        /// Gives input the focus to the object
        void giveFocus();


        virtual void setup() override = 0;
        virtual void update(float delta, GameActions &ga) override;
        virtual void draw(GameActions &ga) override;


        /// Adds a UIObject as a subobject of this one
        template <typename T, typename = std::enable_if_t<std::is_base_of<UIObject, T>{}>>
        constexpr void addSubobject(T &&o) {
            static_assert(!std::is_abstract<T>::value, "The type cannot be an abstract class");
            subObjects.push_back(new T(std::forward<T>(o)));
            subObjects.back()->setup();
        }

        /// Deletes the subobject. **Deletes must be done in the update method**.
        template <typename T, typename = std::enable_if_t<std::is_base_of<UIObject, T>{}>>
        constexpr void deleteSubobject(const T *o) {
            static_assert(!std::is_abstract<T>::value, "The type cannot be an abstract class");
            auto it = std::find(subObjects.begin(), subObjects.end(), [&o] (auto &b) { return o == b; });
            if(it != subObjects.end()) {
                delete *it;
                subObjects.erase(it);
            }
        }

        virtual void saveState(json &j) const override;
        virtual void restoreState(const json &j) override;

        virtual ~UIObject();

    };

    constexpr FontStyle operator&(FontStyle a, FontStyle b) {
        return static_cast<FontStyle>(static_cast<int>(a) & static_cast<int>(b));
    }

    constexpr FontStyle operator|(FontStyle a, FontStyle b) {
        return static_cast<FontStyle>(static_cast<int>(a) | static_cast<int>(b));
    }

    constexpr FontStyle operator&=(FontStyle &a, FontStyle b) {
        return a = (a & b);
    }

    constexpr FontStyle operator|=(FontStyle &a, FontStyle b) {
        return a = (a | b);
    }


    /// A piece of text
    class UILabel: public UIObject {
    public:
        Optional<Color> backgroundColor;
        Optional<Color> borderColor;

        UILabel(UIObject* parent, const glm::vec2 &pos, const std::string &name): UIObject(parent, pos, name) {}
        virtual void setup() override {}
        virtual void draw(GameActions &ga) override;
        virtual void saveState(json &j) const override;
        virtual void restoreState(const json &j) override;
    };

}
