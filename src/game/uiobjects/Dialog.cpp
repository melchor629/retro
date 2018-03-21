#include "Dialog.hpp"
#include <GameActions.hpp>
#include <Game.hpp>
#include <Level.hpp>
#include <glm/vec2.hpp>
#include <utf8.h>

using namespace hw;
using namespace glm;

void Dialog::prepareAnimation() {//2
    auto &text = paginas[pagina];
    size_t characters = utf8::distance(text.begin(), text.end());
    aparecer.clear();
    aparecer.push_back(delay<size_t>(0.4));
    aparecer.push_back(Animation<size_t>(interpolator::Linear<>(),
                                         characters * 0.06,
                                         0,
                                         characters,
                                         [this, &text] (auto p) {
                                             auto it = text.begin();
                                             utf8::advance(it, p, text.end());
                                             if(it != text.end()) {
                                                 utf8::next(it, text.end());
                                                 setText(text.substr(0, it - text.begin()));
                                             } else {
                                                 setText(text);
                                             }
                                         }));
}

void Dialog::setup() {
    auto window = game().getWindow();
    setFont("Ubuntu-R.ttf", 24);
    setTextBoxLimit(BoxLimit::FixedWidth, { window.getSize().x * 0.4 / window.getScaleFactor(), window.getSize().y * 0.3 });
    setAlign(TextHorizontalAlign::Center);
    setAlign(TextVerticalAlign::Center);
    giveFocus();
    goToPage(0);
}

void Dialog::update(float delta, GameActions &ga) {
    UIObject::update(delta, ga);
    if(!aparecer.isCompleted()) aparecer.animate(delta);
}

void Dialog::addPage(std::string &&str) {
    paginas.push_back(std::forward<std::string>(str));
}

void Dialog::nextPage() {
    goToPage(pagina + 1);
}

void Dialog::prevPage() {
    goToPage(pagina - 1);
}

void Dialog::goToPage(size_t page) {
    if(page < paginas.size()) {
        pagina = page;
        prepareAnimation();
        setText("");
    }
}

bool Dialog::hasNextPage() {
    return pagina < paginas.size() - 1;
}

void Dialog::draw(GameActions &ga) {
    ga.fillRectangle({ { 0, 0 }, getTextSize() }, 0x33333377_rgba);
    renderText(ga);
    if(aparecer.isCompleted() && pagina < paginas.size() - 1) {
        vec2 pos = frame.pos + frame.size;
        for(int i = 0; i < 5; i++)
            ga.drawLine(pos - vec2(10+i, 20), pos - vec2(5+i, 15), 0xfafafa_rgb);
        for(int i = 0; i < 5; i++)
            ga.drawLine(pos - vec2(5+i, 15), pos - vec2(10+i,  5), 0xfafafa_rgb);
    }
    UIObject::draw(ga);
}

void Dialog::keyUp(int scancode) {
    UIObject::keyUp(scancode);
    if(scancode == SDL_SCANCODE_SPACE || scancode == SDL_SCANCODE_RETURN) {
        if(!aparecer.isCompleted()) {
            aparecer.complete();
        } else {
            if(hasNextPage()) {
                nextPage();
            } else {
                level.deleteObject(this);
            }
        }
    }
}

void Dialog::mouseUp(const ivec2 &pos, int button, int clicks) {
    UIObject::mouseUp(pos, button, clicks);
    if(button == SDL_BUTTON_LEFT) {
        if(!aparecer.isCompleted()) {
            aparecer.complete();
        } else {
            if(hasNextPage()) {
                nextPage();
            } else {
                level.deleteObject(this);
            }
        }
    }
}
