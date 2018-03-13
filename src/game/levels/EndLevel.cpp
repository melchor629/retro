#include "EndLevel.hpp"

using namespace hw;

void EndLevel::setup() {
    logo = Image::loadImage("hwl.png", game());
    logo->generateAndDestroy();

    tl.add(AnimationChain<float>({
        delay<float>(1),
        Animation<float>(interpolator::CubicIn<>(),
                         1,
                         255.0f,
                         0,
                         [this] (const float &f) { fadeInAlpha = f; })
    }));
    tl.addAfter(0, AnimationChain<float>({
        delay<float>(0.3),
        Animation<float>(interpolator::ElasticOut<>(),
                        1,
                        0.01f,
                        0.92f,
                        [this] (auto f) { scale = f; })
    }));
    tl.addAfter(1, AnimationChain<bool>({
        delay<bool>(0.4),
        Animation<bool>(interpolator::Linear<>(),
                        0.02, false, true, [this] (auto b) { showText = b; })
    }));
}

void EndLevel::update(float delta) {
    if(!tl.isCompleted()) {
        tl.animate(delta);
    } else {
        constexpr float π = 3.1415926536f;
        textColor.r = 190 + 64*sin(2 * π * textColorPhase / 2 - π/2);
        textColor.g = 190 + 64*cos(2 * π * textColorPhase / 3);
        textColor.b = 190 + 64*sin(2 * π * textColorPhase / 4 + π/2);
        textColor.a = 0xEF;
        ingPoint = pow(sin(2 * π * textColorPhase / 1.5f), 3);
        textColorPhase += delta;
    }
}

bool EndLevel::predraw() {
    ga.clear(0x082932_rgb);
    return true;
}

void EndLevel::draw() {
    const auto canvasSize = ga.canvasSize();
    const vec2 logoSize = vec2(logo->getSize()) * (float(canvasSize.x) / float(logo->getWidth())) * scale;
    const Frame logoFrame = { { (canvasSize.x - logoSize.x) / 2, canvasSize.y / 4 * 3 - logoSize.y / 2 }, logoSize };
    logo->draw(logoFrame);

    if(showText && textColor.a != 0) {
        static const string text1 = "Thanks for";
        static const string text2 = "coming :)";
        auto textSize = ga.sizeOfText(text1);
        ga.print(text1, { (canvasSize.x - textSize.x) / 2, canvasSize.y / 4 - textSize.y / 2 + ingPoint }, textColor);
        textSize = ga.sizeOfText(text2);
        ga.print(text2, { (canvasSize.x - textSize.x) / 2, canvasSize.y / 4 - textSize.y / 2 + textSize.y + 1 + ingPoint }, textColor);
    }

    if(fadeInAlpha > 0.01f) {
        ga.fillRectangle({ { 0, 0 }, canvasSize }, Color(0.f, 0.f, 0.f, round(fadeInAlpha)));
    }
}
