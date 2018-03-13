
#define GAME_INCLUDED 1

#include <Player.hpp>
#include <MapObject.hpp>
#include <GameActions.hpp>
#include <Animation.hpp>
#include <AnimationChain.hpp>
#include <Timeline.hpp>
#include <Image.hpp>
#include <utf8.h>
#include <CollisionDetection.hpp>
#include "Palette.hpp"

////------
class DemoLevel: public Level {
    uint32_t i;
    bool palante = true;
public:

    virtual void setup() override {
        i = 0;
        auto &audio = this->game().getAudio();
#ifndef __linux__
        audio.loadMusic("onestop.mid");
        log.debug("Play %d", audio.playMusic("onestop"));
#else
        audio.loadMusic("sc.flac");
        log.debug("Play %d", audio.playMusic("sc"));
#endif
    }

    virtual void draw() override {
        ga.clear(00000000_rgba);
        const auto size = ga.canvasSize();
        const auto maxy = (size.y / 5);
        const auto maxi = maxy * (size.x / 16);
        for(auto j = 0u; j < std::min(i / 2, maxi); j++) {
            uvec2 pos { (j / maxy) * 16, (j % maxy) * 5 };
            ga.print("fuck", pos, (j % 10) * 8 + 7);
        }
    }

    void windowResized(const glm::ivec2&, const glm::ivec2&) override {
        i = 0;
    }

    virtual void update(float f) override {
        const auto size = ga.canvasSize();
        f = 1 / f; f = 60 / f;
        if(palante) {
            i += std::round(f);
            if(i / 2 != std::min(i / 2, (size.y / 5) * (size.x / 16))) {
                palante = false;
                i = (size.y / 5) * (size.x / 16) * 2;
            }
        } else {
            i -= std::min((uint32_t) std::round(f), i);
            if(i == 0) palante = true;
        }
    }
    virtual void cleanup() override {}


    DemoLevel(Game &game, const char* name): Level(game, name) {}

};

class DemoGame: public Game {
protected:
    virtual void setup() override {
        setPalette(PaletteTest());
        loadFont("8b.ttf");
        addLevel<DemoLevel>("level", true);
    }

    virtual void cleanup() override {}
public:
    DemoGame(const Builder &builder): Game(builder) {}
};
////------

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

#if defined(__ANDROID__) || defined(__IOS__)
    auto g = mobileBuilder<DemoGame>();
#else
    auto g = windowedBuilder<DemoGame>(1280, 720);
#endif
    g->loop();
    delete g;
    return 0;
}
