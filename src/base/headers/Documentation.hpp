/*! \mainpage retro++ documentation
 *
 * \section intro Introduction
 * `retro++` is a simple retro-pixelart 2D game engine that aims to help to introduce
 * anybody with knowledge of programming (specially with C++) into the world of game development
 * without having to deal with the low-level APIs OpenGL or DirectX, or with newer APIs like
 * Vulkan or Metal (both unsupported in this engine).
 *
 * To develop using this engine, you will need the following:
 *
 *  - **Windows**:
 *    - [Visual Studio 2015 or 2017](https://www.visualstudio.com/)
 *    - [CMake](http://cmake.org)
 *  - **Debian/Ubuntu**
 *    - `build-essential` (in Ubuntu is usually installed by default)
 *    - `cmake`
 *    - `libsdl2-dev` / `libsdl2-2.0.0`
 *    - `libsdl2-ttf-dev` / `libsdl2-ttf-2.0.0`
 *    - `libsdl2-mixer-dev` / `libsdl2-mixer-2.0.0` (recommended to check if `timidity` is installed)
 *    - `libsdl2-image` / `libsdl2-image-2.0.0`
 *    - An IDE/editor of your election. You can use [Atom](https://atom.io), [Visual Studio Code](https://www.visualstudio.com/), Geany, [Sublime Text](http://sublimetext.com), [CLion](https://www.jetbrains.com/clion/)...
 *    - A terminal if the IDE/editor doesn't compile the project for you
 *  - **Arch Linux based distro**
 *    - `sdl2`
 *    - `sdl2_ttf`
 *    - `sdl2_mixer` (see [_Midi on Linux_](#midi_on_linux))
 *    - `sdl2-image`
 *    - `cmake`
 *    - An IDE/editor of your election. You can use [Atom](https://atom.io), [Visual Studio Code](https://www.visualstudio.com/), Geany, [Sublime Text](http://sublimetext.com), [CLion](https://www.jetbrains.com/clion/)...
 *    - A terminal if the IDE/editor doesn't compile the project for you
 *  - **macOS**
 *    - [CMake](http://cmake.org)
 *    - You can use Xcode (easy to use), or any of the programs listed in Debian/Ubuntu/Arch Linux.
 *
 * The engine has included some tools to help you to make the resources of the game easily.
 * The editor can be found in retro::Editor. Create an instance of this game and then call
 * retro::Editor::setPalette() and retro::Editor::setFont() and you got the editor running.
 * It is recommended to use a window of 1280x720 always, not more, not less.
 *
 * \section howto Getting started with the engine
 * To start doing a game, first you must understand how the engine works and what do you need to
 * make a game.
 *
 *  - Everything you need is under {@link retro} namespace.
 *  - A game is represented by your implementation of retro::Game. You must extend retro::Game
 * and add everything you need.
 *  - A game will contain levels. A retro::Level is just like a container for a world, a map,
 * a settings menu or something like this. You decide what stuff will contain a level. A level
 * is created by extending retro::Level and adding it to the retro::Game with
 * retro::Game::addLevel().
 *  - One level will contain {@link retro::Object}s placed around the level, optionally a
 * retro::MapObject and, also optionally, a HUD (drawn inside retro::Level::draw()).
 * {@link retro::Object}s are added with retro::Level::addObject(). Also, you can be notified
 * when some event occurs (like a key press or mouse movement), done by overriding a list of
 * methods annotated in the retro::Level documentation.
 *  - retro::Object, retro::MovableObject, retro::Player and retro::ControlledPlayer are the
 * basic types for implementing objects. See their documentation pages to know what they do.
 *  - To make cool objects, you can use retro::Sprite (from a retro::Sprites file) to draw a
 * graphic representation of it, without having to draw manually the object.
 *  - You can even make a HUD with retro::UIObject. Make objects by inheriting retro::UIObject
 * and add them into the retro::Level or inside another retro::UIObject, making a tree of
 * retro::UIObject. _(more below)_
 *  - But, *how do I create an instance of my implementation of the retro::Game?* You will use
 * retro::Game::Builder to create the game and the window (using the [Bulder pattern](https://en.wikipedia.org/wiki/Builder_pattern))
 * to help you create the window and the game.
 *
 * Another thing important you must know is the engine uses two basic dependencies, and one of
 * them is very useful for game development in C++: [glm](http://glm.g-truc.net). You should
 * check their documentation for discovering how to use it, but I will introduce you a bit.
 * `glm` is a mathemathical library based on OpenGL Shaders (GLSL). A game without maths is not
 * a game, and it's cool if you use a good library for maths. In general, you will use 2D vectors
 * and 4D vectors (for colours) _but retro::Color is better for that_. So you must include the corresponding headers
 * `#include <glm/vec2.hpp>` and `#include <glm/vec4.hpp>`. Both have 3 basic implementations:
 * one using floats (`glm::vec2` and `glm::vec4`), one using ints (`glm::ivec2` and `glm::ivec4`)
 * and one using unsigned ints (`glm::uvec2` and `glm::uvec4`). In the engine, different of this
 * types of vectors are used to simplify the programming. Basically because you can add two
 * vectors using `+` operator (or substract) and multiply (or divide) by a scalar of the
 * corresponding type easily. Let's see an example (inside an retro::Level::update() method):
 *
 * ```
 * using namespace std; //#include <algorithm>
 * using namespace glm;
 * ...
 * DerivedControlledPlayerType player = getObjectByName<DerivedControlledPlayerType>("player");
 * MapObject map = getObjectByName<MapObject>("map");
 * ga.camera(max(vec2{0,0}, min(player.getFrame().pos - vec2(ga.canvasSize() / 2u), map.getFrame().size)));
 * ```
 *
 * This code follows (using std::min and std::max, glm exposes their version, but are not needed)
 * the player, positioning it in the center of the screen, but if the camera will be out of the
 * map bounds, it stops from moving in that axis (or both axis). So the camera is always inside
 * the map.
 *
 * You can play any kind of music and sound in your game. In general, supported audio formats
 * are `wav`, `aiff`, `voc`, `mod`, `midi` (in Linux you need `timidity` as suggested before),
 * `ogg/vorbis`, `mp3` and `flac` (on Windows you need to provide some `dll`s). Probably some
 * other formats are supported too. See the debug log when running the engine. There are two
 * different types of audio: samples (aka effects or little sounds) and music. For any of these
 * types, there's methods inside retro::Game::Audio class that you can use to load, play and
 * stop. For effects, you can play a variety of them in the same time because uses channels. By
 * default, there's 8 channels, but you can change it in any moment. Music cano only be played
 * one at any time. In general you will only play one music piece at the same moment. Read the
 * documentation of retro::Game::Audio to know every option you have. Also in a retro::Game, you
 * have accessible the variable retro::Game::audio, and for retro::Level, you have also the same
 * retro::Level::audio variable. **TIP** If you want to make retro music, you have the webpage
 * http://www.beepbox.co/, and allows you to export the music in `wav` or `midi`. You can use
 * https://jfxr.frozenfractal.com to make sound effects, and exports audio to `wav`.
 *
 * There's UI elements that inherit from retro::Object. These objects have some special abilities.
 * Read a bit about the documentation about retro::UIObject. In most cases, you will implement
 * nice UI elements by drawing some stuff on it and, maybe, draw some non-pixelated text. The
 * retro::UIObject is drawn with 100% of resolution, so your HUDs will look nice. They receive
 * input events, like retro::Level does, and the user could have some interaction with them.
 * But for text input, the treatment is different. First ensure that you have called
 * retro::GameActions::startInputText() (with the edit region), then you will receive events
 * for characters pressed and _composition_ text, everything in UTF-8. To delete a character,
 * you need to do something when backspace key is up and delete the character using [utfcpp](https://github.com/nemtrif/utfcpp)
 * library. As an example of that, you can see retro::UIObject::charKey() and retro::UIObject::textEdit().
 * For concluding, the UIObjects can add another UIObjects and form an UI hierarchy (like in most
 * desktop GUI kits).
 *
 * For manipulating UTF-8 strings, the engine incorporates [utfcpp](https://github.com/nemtrif/utfcpp),
 * as I mentioned before. This library has a bunch of utilities that helps the programmer to treat
 * UTF-8 with ease. Maybe is not a huge library, but works well with `std::string` and even with
 * C string `char*`.
 *
 * With this clear, now you can start making your game. Let's see what you can accomplish :)
 *
 * \section midi_on_linux MIDI on Linux
 * It's difficult to make effective MIDI playing on Linux, but I will try to do my best.
 * First install _timidity_ and a Sound Font (on Arch Linux based `timidity++` and
 * `timidity-freepats`). After that, configure timidity to use the new sound font, i.e. on Arch
 * Linux should modify `/etc/timidity++/timidity.cfg` and append `soundfount
 * /usr/share/soundfonts/timidity-freepats.sf2`.
 *
 * [See this page](https://wiki.archlinux.org/index.php/Timidity), if you want :)
 *
 * \section rlpg Related pages
 *  - \ref inspapi Inspection API
 **/

/** \page inspapi Inspection API
 * \tableofcontents
 * The engine has a simple, but yet effective Inspection API that allows you to modify some values
 * in runtime using an external program. This is done through the save and restore API for levels
 * and objects.
 *
 * \section intf API Interface
 * The API uses JSON to send and receive data, the same format used to _save the game_ and the
 * `.save` file. In this section, I will show the interfaces used in the API.
 *
 * The interfaces "language" is just JSON for everything except for the values. The values are
 * the type expected in the key. In general, shows the structure of the JSON and the types
 * required. A question mark `?` at the end of a type means that can be `null`. `Any` means
 * that can be any type of JSON type.
 *
 * \subsection intf-req Request Interface
 * A request to the API is done via an Array of " _commands_ ". Let's see the command interface:
 *
 * ```
 * {
 *   "command": String,
 *   "value": Any?
 * }
 * ```
 *
 * A request will be an array of commands. If you don't send an array, the API will return with
 * an error (see below section \ref intf-res-error "Error" for errors).
 *
 * The `command` attribute accesses an attribute using a path separated by `::`. Imagine that you
 * want to access the `currentLevel` in the `game` object. You will write `game::currentLevel`.
 * For accessing arrays, write the position as attribute name: `game::levels::0::name`.
 *
 * The `value` attribute, if set, modifies the attribute with the value set. When the value is
 * an object, will modify only the values that are inside the original attribute (instead of
 * replace it), this way you can modify a subset of the attribute without having to include all
 * the original value, and avoids corrupting the game. If the value is null, anything can be put
 * there. Make sure the thing you put there is correct, because you can make the engine to crash
 * if incorrect values are put there. Array values cannot be modified directly, only element by
 * element.
 *
 * \subsection intf-res Response
 * The response is an array too. For every command the client sends, the API will send it back
 * a response, one by one. The response can be an error for that command, and options or a value.
 * If the request was bad, only one item will be inside the array, and it will be of type error too.
 *
 * \subsection intf-res-opt Options response interface
 * When the command accesses an object or an array and a value is not set, it will return an options
 * object. Has the following structure in case of object:
 *
 * ```
 * {
 *   "options": [ { "attribute": String, "type": String }, ... ],
 *   "values": Any?
 * }
 * ```
 *
 * And in case of an array:
 *
 * ```
 * {
 *   "elements": Number,
 *   "values": Any?
 * }
 * ```
 *
 * `options` attribute shows the attributes available inside, for the object one by one, for the array
 * the number of elements available. `values` attribute could show the value of the object, if set.
 *
 * \subsection intf-res-value Value response interface
 * If the command request accesses and primitive value, of involves a changing the object, then the
 * value of the attribute itself will be returned. If the value has changed, it will return that new
 * value. The interface iself is not available because depends on the C++ type.
 *
 * \subsection intf-res-error Error response interface
 * In case of error, the Inspection API will return the following:
 *
 * ```
 * {
 *   "error": String,
 *   "detailed": String?
 * }
 * ```
 *
 * `error` will contain a description of what is wrong. `detailed` will appear if an error occurred
 * in the transport layer or when a request cannot be parsed from JSON.
 *
 * \section app-layer Application Layer
 * Well, the [Application Layer](https://en.wikipedia.org/wiki/OSI_model) is very simple: _the json itself
 * forms the layer_. A request is done, a json is sent to the server. A response is made, a json is sent
 * to the client. The connection closes here.
 *
 * \section transp-layer Transport Layer
 * To connect to the Inspection API, the client has to connect to `tcp://127.0.0.1:32145` or
 * `tcp://[::1]:32145`. In general `tcp://localhost:31245` is a good option.
 *
 * The server listens only for loopback device for IPv4 and IPv6 on port 31245 using TCP.
 *
 * \section example Examples of request-response
 * **Recommendation**: Keep [this page](http://json.parser.online.fr/beta/) open while checking the examples.
 *
 * The examples are easy to understand: first the request, second the response.
 * \subsection ex-empty Empty request
 * ```
 * []
 * ```
 * ```
 * [{"options":[{"attribute":"game","type":"Object"}]}]
 * ```
 *
 * \subsection ex-game Object attribute without value request
 * ```
 * [{"command": "game", "value": null}]
 * ```
 * ```
 * [{"options":[{"attribute":"currentLevel","type":"Object"},{"attribute":"levels","type":"Array"},{"attribute":"name","type":"String"},{"attribute":"path","type":"String"},{"attribute":"quit","type":"Bool"}]}]
 * ```
 *
 * \subsection ex-error Attribute access with error
 * ```
 * [{"command": "game::levels::7", "value": null}]
 * ```
 * ```
 * [{"error":"Level '7' not found"}]
 * ```
 *
 * \subsection ex-obj Object attribute with value request
 * ```
 * [{"command": "game::currentLevel", "value": null}]
 * ```
 * ```
 * [{"options":[{"attribute":"cameraPos","type":"Object"},{"attribute":"lastColor","type":"Object"},{"attribute":"name","type":"String"},{"attribute":"objects","type":"Array"},{"attribute":"uiObjects","type":"Array"}],"values":{"cameraPos":{"x":0.0,"y":28.0},"lastColor":{"a":255,"b":255,"g":255,"r":255},"name":"level","objects":[{"frame":{"pos":{"x":0.0,"y":0.0},"size":{"x":1024.0,"y":2048.0}},"name":"sitt"},{"acceleration":{"x":0.0,"y":0.0},"frame":{"pos":{"x":64.0,"y":64.0},"size":{"x":3.0,"y":3.0}},"keys":{"down":22,"left":4,"right":7,"up":26},"name":"player","playerSpeed":30.0,"speed":{"x":0.0,"y":0.0}},{"frame":{"pos":{"x":15.0,"y":15.0},"size":{"x":8.0,"y":8.0}},"name":"obstaculo"}],"uiObjects":[{"boxLimit":1,"color":{"a":255,"b":255,"g":106,"r":149},"font":{"outline":0,"path":"/Users/melchor9000/Desktop/retro++/res/Ubuntu-R.ttf","size":26,"style":0},"frame":{"pos":{"x":792.60546875,"y":98.52110290527344},"size":{"x":300.0,"y":300.0}},"horizontalAlign":1,"name":"untexto","subObjects":[{"backgroundColor":null,"borderColor":null,"boxLimit":2,"color":{"a":255,"b":255,"g":255,"r":255},"font":{"outline":0,"path":"/Users/melchor9000/Desktop/retro++/res/Ubuntu-R.ttf","size":11,"style":0},"frame":{"pos":{"x":10.0,"y":10.0},"size":{"x":148.0,"y":14.0}},"horizontalAlign":0,"name":"otroTexto","subObjects":[],"text":"Otro texto normal y corriente","textFrame":{"x":148,"y":14},"verticalAlign":0}],"text":"Un texto normal y cualquiera\nSi xD","textFrame":{"x":300,"y":300},"verticalAlign":1}]}}]
 * ```
 *
 * \subsection ex-value1 Change value
 * ```
 * [{"command": "game::currentLevel::uiObjects::0::text", "value": "Hola"}]
 * ```
 * ```
 * ["Hola"]
 * ```
 *
 * \subsection ex-value2 Change value of an object (partially)
 * ```
 * [{"command": "game::currentLevel::uiObjects::0", "value": {"color": {"r": 10}}}]
 * ```
 * ```
 * [{"boxLimit":1,"color":{"a":255,"b":255,"g":254,"r":10},"font":{"outline":0,"path":"/Users/melchor9000/Desktop/retro++/res/Ubuntu-R.ttf","size":26,"style":0},"frame":{"pos":{"x":800.0,"y":100.0},"size":{"x":300.0,"y":300.0}},"horizontalAlign":1,"name":"untexto","subObjects":[{"backgroundColor":null,"borderColor":null,"boxLimit":2,"color":{"a":255,"b":255,"g":255,"r":255},"font":{"outline":0,"path":"/Users/melchor9000/Desktop/retro++/res/Ubuntu-R.ttf","size":11,"style":0},"frame":{"pos":{"x":10.0,"y":10.0},"size":{"x":148.0,"y":14.0}},"horizontalAlign":0,"name":"otroTexto","subObjects":[],"text":"Otro texto normal y corriente","textFrame":{"x":148,"y":14},"verticalAlign":0}],"text":"Hola","textFrame":{"x":300,"y":300},"verticalAlign":1}]
 * ```
 *
 * \subsection ex-multiple-cmd Multiple commands in one request
 * ```
 * [{"command": "game::currentLevel::uiObjects::0::text", "value": null}, {"command": "game::currentLevel::uiObjects::0::font::size", "value": 15}]
 * ```
 * ```
 * ["Hola",15]
 * ```
 */
