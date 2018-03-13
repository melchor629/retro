#include <UIObject.hpp>
#include <Game.hpp>
#include <utf8.h>
#include <unordered_map>
#include <locale>

#if !defined(_WIN32) and !defined(__ANDROID__) && !defined(__IOS__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#if defined(__APPLE__) && defined(__MACH__) && !defined(__IOS__)
#include <SDL2_ttf/SDL_ttf.h>
#else
#if !defined(_WIN32) and !defined(__ANDROID__) && !defined(__IOS__)
#include <SDL2/SDL_ttf.h>
#else
#include <SDL_ttf.h>
#endif
#endif

using namespace std;
using namespace retro;
using namespace glm;

struct CacheValue {
    vector<ivec2> linesSize;
    vector<SDL_Texture*> linesSurface;
    vector<string> linesText;

    void clear() {
        for(auto &t: linesSurface) if(t != nullptr) SDL_DestroyTexture(t);
    }
};


void UIObject::clearCache() {
    if(cacheValue != nullptr) {
        cacheValue->clear();
        delete cacheValue;
        cacheValue = nullptr;
    }
}

UIObject::UIObject(Game &game, Level &level, const vec2 &pos, const string &name): Object(game, level, pos, name) {
    font = nullptr;
}


UIObject::UIObject(UIObject* parent, const glm::vec2 &pos, const std::string &name): UIObject(parent->game(), parent->level, pos, name) {
    this->parent = parent;
    renderer = parent->renderer;
    gamePath = parent->gamePath;
}

UIObject::~UIObject() {
    clearCache();
    if(font != nullptr) TTF_CloseFont(font);
    for(auto* &o: subObjects) delete o;
    if(parent) {
        if(parent->focused == this) {
            lostFocus();
            parent->focused = nullptr;
            parent->focus();
        }
    } else {
        if(level.focused == this) {
            lostFocus();
            level.focused = nullptr;
        }
    }
}

void UIObject::setTextBoxLimit(BoxLimit limit, const ivec2 size) {
    if(limit == BoxLimit::FixedWidthAndHeight) {
        textFrame = size;
    } else if(limit == BoxLimit::FixedWidth) {
        textFrame.x = size.x;
        //Use a minimum height
        textFrame.y = std::max(TTF_FontLineSkip(font), size.y);
    } else if(limit == BoxLimit::Nothing) {
        textFrame.x = std::max(0, size.x);
        textFrame.y = std::max(TTF_FontLineSkip(font), size.y);
    }
    clearCache();
    this->boxLimit = limit;
}

BoxLimit UIObject::getTextBoxLimit() {
    return boxLimit;
}

void UIObject::setFontWithPath(const string &path, uint32_t size) {
    if(fontPath == path && fontSize == size) return;
    if(font != nullptr) TTF_CloseFont(font);
    font = TTF_OpenFont(path.c_str(), int(size));
    if(font == nullptr) {
        throw runtime_error(string("Could not open font ") + path + ": " + TTF_GetError());
    }
    fontPath = path;
    fontSize = size;
    clearCache();
}

void UIObject::setFont(const string &name, uint32_t size) {
    setFontWithPath(gamePath + name, size);
}

string UIObject::getFontName() {
    return TTF_FontFaceFamilyName(font);
}

uint32_t UIObject::getFontSize() {
    return fontSize;
}

void UIObject::setFontStyle(FontStyle style) {
    int _style = 0;
    using f = FontStyle;
    if((style & f::Bold) == f::Bold) _style |= TTF_STYLE_BOLD;
    if((style & f::Italic) == f::Italic) _style |= TTF_STYLE_ITALIC;
    if((style & f::Underline) == f::Underline) _style |= TTF_STYLE_UNDERLINE;
    if((style & f::Strikethrough) == f::Strikethrough) _style |= TTF_STYLE_STRIKETHROUGH;
    if(TTF_GetFontStyle(font) != _style) {
        TTF_SetFontStyle(font, _style);
        clearCache();
    }
}

FontStyle UIObject::getFontStyle() {
    using f = FontStyle;
    f _style = f::Normal;
    int style = TTF_GetFontStyle(font);
    if((style & TTF_STYLE_BOLD)) _style |= f::Bold;
    if((style & TTF_STYLE_ITALIC)) _style |= f::Italic;
    if((style & TTF_STYLE_UNDERLINE)) _style |= f::Underline;
    if((style & TTF_STYLE_STRIKETHROUGH)) _style |= f::Strikethrough;
    return _style;
}

void UIObject::setFontOutline(uint32_t outlinepx) {
    if(uint32_t(TTF_GetFontOutline(font)) != outlinepx) {
        TTF_SetFontOutline(font, int(outlinepx));
        clearCache();
    }
}

uint32_t UIObject::getFontOutline() {
    return TTF_GetFontOutline(font);
}

void UIObject::setText(const string &str) {
    if(text != str) {
        text = str;
        replace_if(text.begin(),
                   text.end(),
                   [loc=locale("")] (auto &c) { return iscntrl(c, loc) && c != '\n'; },
                   ' ');
        clearCache();
    }
}

void UIObject::setTextColor(Color c) {
    if(c != color) {
        color = c;
        clearCache();
    }
}

static auto split = [] (const string &str, auto delim) -> vector<string> {
    vector<string> parts;
    size_t pos, opos = 0;
    while((pos = str.find(delim, opos)) != string::npos) {
        parts.push_back(str.substr(opos, pos - opos));
        opos = pos + strlen(delim);
    }
    if(opos < str.size()) parts.push_back(str.substr(opos));
    return parts;
};

void UIObject::generateCache() {
    using l = BoxLimit;
    CacheValue value;
    SDL_Color col = { uint8_t(color.r), uint8_t(color.g), uint8_t(color.b), uint8_t(color.a) };
    auto textWidth = [this] (const std::string &t) -> size_t { int w; TTF_SizeUTF8(font, t.c_str(), &w, nullptr); return size_t(w); };
    if(boxLimit == l::FixedWidthAndHeight || boxLimit == l::FixedWidth) {
        int w, h;
        for(auto &originalLine: split(text, "\n")) {
            TTF_SizeUTF8(font, originalLine.c_str(), &w, &h);
            float parts = float(textFrame.x) / float(w);
            auto loc = locale("");
            auto lastPos = originalLine.begin();
            size_t numOfChars = utf8::distance(originalLine.begin(), originalLine.end());
            while(lastPos < originalLine.end() && (boxLimit == l::FixedWidth || value.linesText.size() * TTF_FontLineSkip(font) < textFrame.y)) {
                auto nextPos = lastPos;
                utf8::advance(nextPos,
                              std::min(size_t(utf8::distance(originalLine.begin(), originalLine.end()) * parts), numOfChars),
                              originalLine.end());
                auto it = nextPos;
                auto nextPosSi = nextPos;
                //If the character is not a space
                //The current position is greater than the start position for the substring
                //The current position is less than the size of the text
                //OR the textWith is higher than the box
                while((it > lastPos && it < originalLine.end() && !std::isspace(*it, loc)) || textWidth(originalLine.substr(lastPos - originalLine.begin(), it - lastPos)) > textFrame.x) {
                    utf8::prior(it, originalLine.begin());
                    nextPos = it;
                }
                //If we cannot split, let the whole line be there
                if(nextPos == lastPos) {
                    nextPos = nextPosSi;
                    while(textWidth(originalLine.substr(lastPos - originalLine.begin(), it - lastPos)) > textFrame.x) {
                        utf8::prior(it, originalLine.begin());
                        nextPos = it;
                    }
                }
                string line = originalLine.substr(lastPos - originalLine.begin(), nextPos - lastPos);
                value.linesText.push_back(line);
                SDL_Surface* surface = TTF_RenderUTF8_Blended(font, line.c_str(), col);
                if(surface != nullptr) {
                    value.linesSurface.push_back(SDL_CreateTextureFromSurface(renderer, surface));
                    value.linesSize.push_back({ surface->w, surface->h });
                    SDL_FreeSurface(surface);
                } else {
                    value.linesSurface.push_back(nullptr);
                    value.linesSize.push_back({ 0, TTF_FontHeight(font) });
                }
                numOfChars -= utf8::distance(lastPos, nextPos);
                lastPos = nextPos;
                while(lastPos < originalLine.end() && std::isspace(*lastPos, loc)) {
                    lastPos++;
                    numOfChars--;
                }
            }
        }

        if(boxLimit == l::FixedWidth) {
            textFrame.y = std::max(value.linesText.size() * TTF_FontLineSkip(font), size_t(textFrame.y));
        }
    } else if(boxLimit == l::Nothing) {
        uvec2 textFrame = { 0, 0 };
        for(auto &originalLine: split(text, "\n")) {
            SDL_Surface* surface = TTF_RenderUTF8_Blended(font, originalLine.c_str(), col);
            textFrame.x = std::max(uint32_t(surface->w), textFrame.x);
            textFrame.y += surface->h;
            value.linesText.push_back(originalLine);
            value.linesSurface.push_back(SDL_CreateTextureFromSurface(renderer, surface));
            value.linesSize.push_back({ surface->w, surface->h });
            SDL_FreeSurface(surface);
        }
        this->textFrame.x = std::max(textFrame.x, this->textFrame.x);
        this->textFrame.y = std::max(textFrame.y, this->textFrame.y);
    }
    cacheValue = new CacheValue(value);
}

void UIObject::renderText(GameActions &ga, const vec2 &pos) {
    using l = BoxLimit;
    if(cacheValue == nullptr) {
        generateCache();
    }

    frame.size.x = std::max(frame.size.x, float(pos.x + textFrame.x));
    frame.size.y = std::max(frame.size.y, float(pos.y + textFrame.y));

    CacheValue &value = *cacheValue;
    int left = int(-ga.l.cameraPos.x + pos.x);
    SDL_Rect rect = {
        left,
        int(-ga.l.cameraPos.y + pos.y),
        0,
        0
    };
    if(vAlign == TextVerticalAlign::Center) {
        int diff = int(textFrame.y - TTF_FontLineSkip(font) * value.linesSize.size()) / 2;
        rect.y += diff;
    } else if(vAlign == TextVerticalAlign::Bottom) {
        rect.y += int(textFrame.y - TTF_FontLineSkip(font) * value.linesSize.size());
    }

    for(size_t i = 0; i < value.linesText.size(); i++) {
        rect.w = value.linesSize[i].x;
        rect.h = value.linesSize[i].y;

        if(hAlign == TextHorizontalAlign::Center) {
            int diff = int(textFrame.x - rect.w) / 2;
            rect.x = left + diff;
        } else if(hAlign == TextHorizontalAlign::Right) {
            rect.x = int(left + textFrame.x) - rect.w;
        } else {
            rect.x = left;
        }

        if(value.linesSurface[i] != nullptr) {
            if(value.linesText.size() - 1 != i || boxLimit != l::FixedWidthAndHeight) {
                SDL_RenderCopy(renderer, value.linesSurface[i], NULL, &rect);
            } else {
                //That's the last line, we need to clip the text
                int top = rect.y;
                int bottom = int(frame.pos.y) + textFrame.y;
                int diff = std::min(bottom - top, value.linesSize[i].y);
                SDL_Rect fromRect = { 0, 0, rect.w, diff };
                rect.h = diff;
                SDL_RenderCopy(renderer, value.linesSurface[i], &fromRect, &rect);
            }
        }

        rect.y += TTF_FontLineSkip(font);
    }
}

void UIObject::mouseDown(const glm::ivec2 &pos, int button, int clicks) {
    for(auto* &sub: subObjects) {
        if(sub->frame.isInside(pos)) {
            sub->mouseDown(pos - ivec2(sub->frame.pos), button, clicks);
        }
    }

    pressed |= button;
}

void UIObject::mouseUp(const glm::ivec2 &pos, int button, int clicks) {
    auto oldFocused = focused;
    focused = nullptr;
    for(auto* &sub: subObjects) {
        if(sub->frame.isInside(pos)) {
            if(sub->pressed & button) focused = sub;
            sub->mouseUp(pos - ivec2(sub->frame.pos), button, clicks);
        }
    }

    if(pressed & button) {
        mouseClick(button);
        pressed &= ~button;
    }

    if(oldFocused != focused) {
        if(focused != nullptr) {
            focused->focus();
        } else if(isFocused) {
            focus();
        }
        if(oldFocused != nullptr) {
            oldFocused->lostFocus();
        } else if(isFocused) {
            lostFocus();
        }
    }
}

void UIObject::mouseMoved(const glm::ivec2 &pos, const glm::ivec2 &desp) {
    if(!wasInside) {
        wasInside = true;
        mouseEnter();
    }

    for(auto* &sub: subObjects) {
        if(sub->frame.isInside(pos)) {
            sub->mouseMoved(pos - ivec2(sub->frame.pos), desp);
        }
    }
}

void UIObject::update(float delta, GameActions &ga) {
    size_t size = subObjects.size();
    for(size_t i = 0; i < subObjects.size(); i++) {
        auto* &subObject = subObjects[i];
        subObject->update(delta, ga);
        frame.size.x = std::max(frame.size.x, subObject->frame.pos.x + subObject->frame.size.x);
        frame.size.y = std::max(frame.size.y, subObject->frame.pos.y + subObject->frame.size.y);
        if(subObjects.size() < size) {
            i -= (size - subObjects.size());
            size = subObjects.size();
        }
    }

    if(wasInside && !frame.isInside(ga.getMousePosition())) {
        wasInside = false;
        mouseExit();
    }

    if(!isFocused && ga.l.focused == this) {
        isFocused = true;
        if(focused == nullptr) focus();
    } else if(isFocused && ga.l.focused != this) {
        focused = nullptr;
        isFocused = false;
        if(focused) focused->lostFocus(); else lostFocus();
    }
}

void UIObject::draw(GameActions &ga) {
    for(auto* &subObject: subObjects) {
        auto old = ga.l.cameraPos;
        ga.l.cameraPos -= subObject->getFrame().pos;
        subObject->draw(ga);
        ga.l.cameraPos = old;
    }
}

void UIObject::saveState(json &j) const {
    Object::saveState(j);
    j["textFrame"] = textFrame;
    j["boxLimit"] = static_cast<int>(boxLimit);
    j["text"] = text;
    j["color"] = color;
    j["verticalAlign"] = static_cast<int>(vAlign);
    j["horizontalAlign"] = static_cast<int>(hAlign);
    j["font"] = {
        { "style", TTF_GetFontStyle(font) },
        { "outline", TTF_GetFontOutline(font) },
        { "path", fontPath },
        { "size", fontSize }
    };

    j["subObjects"] = json::array();
    for(auto &obj: subObjects) {
        json jj;
        obj->saveState(jj);
        j["subObjects"].push_back(jj);
    }
}

void UIObject::restoreState(const json &j) {
    Object::restoreState(j);
    boxLimit = static_cast<BoxLimit>(static_cast<int>(j["boxLimit"]));
    textFrame = j["textFrame"];
    text = j["text"];
    color = j["color"];
    vAlign = static_cast<TextVerticalAlign>(static_cast<int>(j["verticalAlign"]));
    hAlign = static_cast<TextHorizontalAlign>(static_cast<int>(j["horizontalAlign"]));
    setFontWithPath(j["font"]["path"], j["font"]["size"]);
    TTF_SetFontStyle(font, j["font"]["style"]);
    TTF_SetFontOutline(font, j["font"]["outline"]);
    clearCache();

    Logger log = Logger::getLogger("UIObject#" + name);
    for(auto &obj: j["subObjects"]) {
        std::string objName = obj["name"];
        auto it = std::find_if(subObjects.begin(), subObjects.end(), [objName] (Object *o) { return !strcmp(o->getName(), objName.c_str()); });
        if(it == subObjects.end()) log.warn("The sub-object %s stored in the state doesn't exist. Check your game!", objName.c_str());
        else (*it)->restoreState(obj);
    }
}

void UIObject::giveFocus() {
    if(parent) parent->focused = this;
    else {
        if(game().currentLevel->focused && game().currentLevel->focused != this) {
            game().currentLevel->focused->lostFocus();
        }
        game().currentLevel->focused = this;
    }
    if(focused) focused->lostFocus();
    focused = nullptr;
    isFocused = true;
    focus();
}



void UILabel::draw(GameActions &ga) {
    if(backgroundColor) {
        ga.fillRectangle({ {0,0}, getTextSize() }, *backgroundColor);
    }
    renderText(ga);
    if(borderColor) {
        ga.drawRectangle({ { 0, 0 }, getTextSize() }, *borderColor);
    }
}

void UILabel::saveState(json &j) const {
    UIObject::saveState(j);
    j["backgroundColor"] = backgroundColor;
    j["borderColor"] = borderColor;
}

void UILabel::restoreState(const json &j) {
    UIObject::restoreState(j);
    from_json(j["backgroundColor"], backgroundColor);
    from_json(j["borderColor"], borderColor);
}
