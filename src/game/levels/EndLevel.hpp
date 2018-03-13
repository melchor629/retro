#pragma once

#include <Level.hpp>
#include <Timeline.hpp>
#include <Image.hpp>

namespace hw {

    using namespace std;
    using namespace retro;
    using namespace glm;

    class EndLevel: public Level {
        ImagePtr logo;
        float scale = 0.01f;
        float fadeInAlpha = 255.0f;
        bool showText = false;
        float textColorPhase = 0;
        Color textColor = 0_rgba;
        float ingPoint = 0;

        Timeline tl;

    protected:
        void setup() override;

        void update(float delta) override;

        bool predraw() override;

        void draw() override;

    public:
        EndLevel(Game &game, const char* name): Level(game, name) {}

        void saveState(json &object) const override {
            Level::saveState(object);
            object["scale"] = scale;
            object["fadeInAlpha"] = fadeInAlpha;
            object["showText"] = showText;
            object["textColorPhase"] = textColorPhase;
            object["textColor"] = textColor;
            object["ingPoint"] = ingPoint;
        }

        void restoreState(const json &object) override {
            Level::restoreState(object);
            scale = object["scale"];
            fadeInAlpha = object["fadeInAlpha"];
            showText = object["showText"];
            textColorPhase = object["textColorPhase"];
            textColor = object["textColor"];
            ingPoint = object["ingPoint"];
        }
    };

}
