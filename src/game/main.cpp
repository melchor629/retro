#include <Game.hpp>
#include <Editor.hpp>

#include "levels/FirstLevel.hpp"
#include "levels/EndLevel.hpp"

#if defined(__ANDROID__) || defined(__IOS__)
#include <SDL_main.h>
#endif

using namespace std;
using namespace retro;
using namespace glm;
using namespace hw;



class HWGame: public Game {
protected:
    void setup() override {
        importPalette("palette.aco");
        loadFont("8b.ttf");
        addLevel<FirstLevel>("firstLevel", true);
        addLevel<EndLevel>("endLevel");
    }

    void cleanup() override {}

public:
    HWGame(const Builder &builder): Game(builder) {}
};



template<class GameType>
static inline GameType* windowedBuilder(int, int);
template<class GameType>
static inline GameType* mobileBuilder();

//#include "DemoGame.inc.hpp"
//#include "shit.inc.hpp"

#ifndef GAME_INCLUDED
int main(int argc, char* argv[]) {
    set_terminate([] () {
        Logger &log = Logger::getLogger("Exception Handler");
        log.error("Uncaught exception");
        try {
            rethrow_exception(current_exception());
        } catch(const exception &e) {
            log.error("%s", e.what());
        }

        exit(9);
    });

    if(argc > 1 && !strcmp("--editor", argv[1])) {
        auto g = windowedBuilder<Editor>(1280, 720);
        g->setFont("8b.ttf");
        g->setPalette("palette.gpl");
        g->loop();
        delete g;
    } else {
#if defined(__ANDROID__) || defined(__IOS__)
        auto g = mobileBuilder<HWGame>();
#else
        auto g = Game::Builder()
            .setSize(1280, 720)
            .setName("HW 5 game - retro++ workshop")
            .changeCanvasMode(Game::CanvasMode::UltraLowSize)
            .setResizable(true)
            .enableAudio()
            .build<HWGame>();
#endif
        g->loop();
        delete g;
    }
    return 0;
}
#endif

template<class GameType>
static inline GameType* windowedBuilder(int width, int height) {
    return Game::Builder().setSize(width, height).enableAudio().build<GameType>();
}

template<class GameType>
static inline GameType* mobileBuilder() {
    auto b = Game::Builder();
    auto width = b.getCurrentDisplayMode()->width;
    auto height = b.getCurrentDisplayMode()->height;
    if(width < height) swap(width, height);
    return b.setSize(width, height).enableAudio().changeCanvasMode(Game::CanvasMode::UltraLowSize).build<GameType>();
}

