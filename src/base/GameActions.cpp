#include <Game.hpp>
#include <GameActions.hpp>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <unordered_map>
#include <chrono>

#if defined(__APPLE__) && defined(__MACH__) and !defined(__IOS__)
#include <SDL2_ttf/SDL_ttf.h>
#else
#if !defined(_WIN32) and !defined(__ANDROID__) and !defined(__IOS__)
#include <SDL2/SDL_ttf.h>
#else
#include <SDL_ttf.h>
#endif
#endif

using namespace retro;
using namespace glm;
using namespace std;

struct TextKey {
    string text;
    Color color;

    inline bool operator==(const TextKey &b) const {
        return text == b.text && color == b.color;
    }

    inline bool operator!=(const TextKey &b) const {
        return !(*this == b);
    }
};

struct TextValue {
    SDL_Surface* surface;
    SDL_Texture* texture;
    chrono::time_point<chrono::system_clock> accessed;

    void free() const {
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
};

namespace std {
    template<>
    struct hash<TextKey> {
        size_t operator()(const TextKey &key) const {
            return hash<string>()(key.text) ^ hash<Color>()(key.color);
        }
    };
}

static unordered_map<TextKey, TextValue> textCache;

static inline TextValue& cache_generate(const string &str, const Color &color, TTF_Font* font, SDL_Renderer* renderer) {
    SDL_Color sdlcolor = { static_cast<Uint8>(color.r), static_cast<Uint8>(color.g), static_cast<Uint8>(color.b), static_cast<Uint8>(color.a) };
    SDL_Surface* surface = TTF_RenderUTF8_Solid(font, str.c_str(), sdlcolor);
    if(surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        return textCache[{ str, color }] = { surface, texture, chrono::system_clock::now() };
    } else {
        throw runtime_error("Could not allocate a texture for the text");
    }
}

static inline TextValue& cache_find(const string &text, const Color &color, TTF_Font* font, SDL_Renderer* renderer) {
    TextKey key { text, color };
    auto it = textCache.find(key);
    if(it != textCache.end()) {
        it->second.accessed = chrono::system_clock::now();
        return it->second;
    } else {
        return cache_generate(text, color, font, renderer);
    }
}

void textCache_clear_all_entries() {
    for_each(textCache.begin(), textCache.end(), [] (const auto &it) { it.second.free(); });
    textCache.clear();
}

void textCache_collect_garbage() {
    auto it = textCache.begin();
    auto now = chrono::system_clock::now();
    while(it != textCache.end()) {
        chrono::duration<double> diff = now - it->second.accessed;
        if(diff.count() >= 4.5) {
            it->second.free();
            it = textCache.erase(it);
        } else {
            ++it;
        }
    }
}

SDL_Rect get_rekt(const vec2 &pos, const vec2 &size, bool doubleIt) {
    int mult = doubleIt ? 2 : 1;
    return {
        mult * static_cast<int>(pos.x < 0 ? round(pos.x) : round(pos.x)),
        mult * static_cast<int>(pos.y < 0 ? round(pos.y) : round(pos.y)),
        mult * static_cast<int>(size.x),
        mult * static_cast<int>(size.y)
    };
}

GameActions::GameActions(Game &g, Level &l): g(g), l(l) {}

void GameActions::clear(const Color &color) {
    SDL_SetRenderDrawColor(g.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(g.renderer);
}

void GameActions::clear(size_t color) {
    if(!g.palette) throw runtime_error("Palette is not set");
    auto color2 = (*g.palette)[color];
    if(!color2) throw runtime_error("Invalid color number");
    this->clear(*color2);
}

void GameActions::setColor(const Color &color) {
    l.lastColor = color;
}

void GameActions::setColor(size_t color) {
    if(g.palette) {
        auto color2 = (*g.palette)[color];
        if(color) this->setColor(*color2);
    }
}

void GameActions::drawRectangle(const Frame &frame) {
    this->drawRectangle(frame, l.lastColor);
}

void GameActions::drawRectangle(const Frame &frame, size_t color) {
    if(!g.palette) throw runtime_error("Palette is not set");
    auto color2 = (*g.palette)[color];
    if(!color2) throw runtime_error("Invalid color number");
    this->drawRectangle(frame, *color2);
}

void GameActions::drawRectangle(const Frame &frame, const Color &color) {
    SDL_Rect rekt = get_rekt(frame.pos - camera(), frame.size, doubleIt);
    SDL_SetRenderDrawColor(g.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(g.renderer, &rekt);
    if(doubleIt) {
        rekt.x += 1;
        rekt.y += 1;
        rekt.w -= 2;
        rekt.h -= 2;
        SDL_RenderDrawRect(g.renderer, &rekt);
    }
}

void GameActions::fillRectangle(const Frame &frame) {
    this->fillRectangle(frame, l.lastColor);
}

void GameActions::fillRectangle(const Frame &frame, size_t color) {
    if(!g.palette) throw runtime_error("Palette is not set");
    auto color2 = (*g.palette)[color];
    if(!color2) throw runtime_error("Invalid color number");
    this->fillRectangle(frame, *color2);
}

void GameActions::fillRectangle(const Frame &frame, const Color &color) {
    SDL_Rect rekt = get_rekt(frame.pos - camera(), frame.size, doubleIt);
    SDL_SetRenderDrawColor(g.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(g.renderer, &rekt);
}

void GameActions::drawLine(const vec2 &ipos, const vec2 &epos) {
    this->drawLine(ipos, epos, l.lastColor);
}

void GameActions::drawLine(const vec2 &ipos, const vec2 &epos, size_t color) {
    if(!g.palette) throw runtime_error("Palette is not set");
    auto color2 = (*g.palette)[color];
    if(!color2) throw runtime_error("Invalid color number");
    this->drawLine(ipos, epos, *color2);
}

void GameActions::drawLine(const vec2 &ipos, const vec2 &epos, const Color &color) {
    //Bresenham's Line Algorithm, extracted from
    //https://github.com/ssloy/tinyrenderer/blob/28b766abe59b8635c912ed78b8a6e938a7ef29f2/main.cpp
    vec2 start = ipos;
    vec2 end = epos;
    bool steep = false;
    if(abs(start.x - end.x) < abs(start.y - end.y)) {
        swap(start.x, start.y);
        swap(end.x, end.y);
        steep = true;
    }
    if(start.x > end.x) {
        swap(start.x, end.x);
        swap(start.y, end.y);
    }
    vec2 diff = end - start;
    int derror2 = abs(diff.y) * 2;
    int error2 = 0;
    int y = start.y;
    for(int x = start.x; x <= end.x; x++) {
        if(steep) {
            putColor({ y, x }, color);
        } else {
            putColor({ x, y }, color);
        }
        error2 += derror2;

        if(error2 > diff.x) {
            y += (end.y > start.y ? 1 : -1);
            error2 -= diff.x * 2;
        }
    }
}

void GameActions::print(const string &str, const vec2 &pos) {
    this->print(str, pos, g.currentLevel->lastColor);
}

void GameActions::print(const string &str, const vec2 &pos, size_t color) {
    if(!g.palette) throw runtime_error("Palette is not set");
    auto color2 = (*g.palette)[color];
    if(!color2) throw runtime_error("Invalid color number");
    this->print(str, pos, *color2);
}

void GameActions::print(const string &str, const vec2 &pos, const Color &color) {
    if(g.font == nullptr) throw runtime_error("Font is not loaded");
    if(str.empty() || !doubleIt) return;
    TextValue& val = cache_find(str, color, g.font, g.renderer);
    SDL_Rect dstrekt { 2*int(floor(pos.x - camera().x)), 2*int(floor(pos.y - camera().y)), val.surface->w, val.surface->h };
    SDL_RenderCopy(g.renderer, val.texture, nullptr, &dstrekt);
}

ivec2 GameActions::sizeOfText(const string &str) {
    if(g.font == nullptr) throw runtime_error("Font is not loaded");
    ivec2 size;
    TTF_SizeUTF8(g.font, str.c_str(), &size.x, &size.y);
    size.x /= 2;
    size.y /= 2;
    return size;
}

void GameActions::putColor(const vec2 &pos) {
    this->putColor(pos, g.currentLevel->lastColor);
}

void GameActions::putColor(const vec2 &pos, size_t color) {
    if(!g.palette) throw runtime_error("Palette is not set");
    auto color2 = (*g.palette)[color];
    if(!color2) throw runtime_error("Invalid color number");
    this->putColor(pos, *color2);
}

void GameActions::putColor(const vec2 &pos, const Color &color) {
    if(doubleIt) this->fillRectangle({ { pos.x, pos.y }, { 1, 1 } }, color);
    else {
        SDL_SetRenderDrawColor(g.renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawPoint(g.renderer, int(floor(pos.x)), int(floor(pos.y)));
    }
}

void GameActions::drawCircle(const vec2 &pos, float radius) {
    this->drawCircle(pos, radius, g.currentLevel->lastColor);
}

void GameActions::drawCircle(const vec2 &pos, float radius, size_t color) {
    if(!g.palette) throw runtime_error("Palette is not set");
    auto color2 = (*g.palette)[color];
    if(!color2) throw runtime_error("Invalid color number");
    this->drawCircle(pos, radius, *color2);
}

void GameActions::drawCircle(const vec2 &pos, float radius, const Color &color) {
    //Extracted from https://www.lexaloffle.com/bbs/?tid=29976
    //by @musurca & @felice
    //Adapted from Lua by @melchor9000
    //Minsky circle
    float x = pos.x + 0.5f;
    float y = pos.y + 0.5f;
    float j = radius, k = 0, rat = 1/radius;
    for(float i = 1; i <= radius*0.785f; i++) {
        k -= rat * j;
        j += rat * k;
        putColor({int(x+j),int(y+k)}, color);
        putColor({int(x+j),int(y-k)}, color);
        putColor({int(x-j),int(y+k)}, color);
        putColor({int(x-j),int(y-k)}, color);
        putColor({int(x+k),int(y+j)}, color);
        putColor({int(x+k),int(y-j)}, color);
        putColor({int(x-k),int(y+j)}, color);
        putColor({int(x-k),int(y-j)}, color);
    }
    putColor({int(x),int(y-radius)}, color);
    putColor({int(x),int(y+radius)}, color);
    putColor({int(x-radius),int(y)}, color);
    putColor({int(x+radius),int(y)}, color);
}

void GameActions::fillCircle(const vec2 &pos, float radius) {
    this->fillCircle(pos, radius, g.currentLevel->lastColor);
}

void GameActions::fillCircle(const vec2 &pos, float radius, size_t color) {
    if(!g.palette) throw runtime_error("Palette is not set");
    auto color2 = (*g.palette)[color];
    if(!color2) throw runtime_error("Invalid color number");
    this->fillCircle(pos, radius, *color2);
}

void GameActions::fillCircle(const vec2 &pos, float radius, const Color &color) {
    //Extracted from https://www.lexaloffle.com/bbs/?tid=29976
    //by @musurca, @felice, and @ultrabrite
    //Adapted from Lua by @melchor9000
    //Minsky circle fill
    float x = pos.x + 0.5;
    float y = pos.y + 0.5;
    float j = radius, k = 0, rat = 1/radius;
    for(float i = 1; i <= radius*0.786; i++) {
        k -= rat * j;
        j += rat * k;
        fillRectangle({ { int(x+j), int(y+k) }, { int(1), int(-2*k+1) } }, color);
        fillRectangle({ { int(x-j), int(y+k) }, { int(1), int(-2*k+1) } }, color);
        fillRectangle({ { int(x-k), int(y-j) }, { int(1), int( 2*j+1) } }, color);
        fillRectangle({ { int(x+k), int(y-j) }, { int(1), int( 2*j+1) } }, color);
    }
    fillRectangle({ { int(x), int(y-radius) }, { int(1), int(2*radius) } }, color);
}

void GameActions::enableClipInRectangle(const Frame &rect) {
    SDL_Rect rekt = get_rekt(rect.pos - camera(), rect.size, doubleIt);
    SDL_RenderSetClipRect(g.renderer, &rekt);
}

void GameActions::disableClipInReactangle() {
    SDL_RenderSetClipRect(g.renderer, nullptr);
}

void GameActions::drTHICC(const retro::Frame &frame, const Color &color) {
    SDL_Rect rekt = { static_cast<int>(frame.pos.x - camera().x*2), static_cast<int>(frame.pos.y - camera().y*2), static_cast<int>(frame.size.x), static_cast<int>(frame.size.y) };
    SDL_SetRenderDrawColor(g.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(g.renderer, &rekt);
}

void GameActions::dlTHICC(const vec2 &ipos, const vec2 &epos, const Color &color) {
    SDL_SetRenderDrawColor(g.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(
       g.renderer,
       ipos.x - camera().x,
       ipos.y - camera().y,
       epos.x - camera().x,
       epos.y - camera().y
   );
}

const glm::ivec2 GameActions::mpTHICC() {
    ivec2 pos;
    SDL_GetMouseState(&pos.x, &pos.y);
    pos.x /= 5;
    pos.y /= 5;
    return pos;
}

const glm::ivec2 GameActions::mpProfiteroles() {
    ivec2 pos;
    SDL_GetMouseState(&pos.x, &pos.y);
    if(abs(g.scaleFactor - 1.0f) > 0.1f) {
        pos.x = float(pos.x) / g.scaleFactor;
        pos.y = float(pos.y) / g.scaleFactor;
    }
    return pos;
}

void GameActions::camera(const glm::vec2 &pos) {
    g.currentLevel->cameraPos = pos;
}

glm::vec2 GameActions::camera() {
    return g.currentLevel->cameraPos;
}

bool GameActions::isKeyPressed(int key) {
    return SDL_GetKeyboardState(nullptr)[key];
}

bool GameActions::isModKeyPressed(int mod) {
    return SDL_GetModState() & mod;
}

bool GameActions::isMousePressed(int key) {
    return SDL_GetMouseState(nullptr, nullptr) & key;
}

int GameActions::getMousePressedKey() {
    return SDL_GetMouseState(nullptr, nullptr);
}

ivec2 GameActions::getMousePosition() {
    ivec2 pos;
    SDL_GetMouseState(&pos.x, &pos.y);
    if(doubleIt) {
        pos.x /= 10;
        pos.y /= 10;
    }
    return pos;
}

std::vector<glm::vec2> GameActions::getTouchPositions() {
    vector<vec2> touches;
    for(int device = 0; device < SDL_GetNumTouchDevices(); device++) {
        int finger = 0;
        while(auto a = SDL_GetTouchFinger(SDL_GetTouchDevice(device), finger++)) {
            touches.push_back(vec2(a->x, a->y));
        }
    }
    return touches;
}

void GameActions::captureMouse(bool capture) {
    SDL_SetRelativeMouseMode((SDL_bool) capture);
}

void GameActions::startInputText(const Frame &editTextRegion) {
    SDL_Rect rekt = { int(editTextRegion.pos.x), int(editTextRegion.pos.y), int(editTextRegion.size.x), int(editTextRegion.size.y) };
    SDL_StartTextInput();
    SDL_SetTextInputRect(&rekt);
}

void GameActions::endInputText() {
    SDL_StopTextInput();
}

const uvec2 GameActions::canvasSize() {
    ivec2 size;
    SDL_GetRendererOutputSize(g.renderer, &size.x, &size.y);
    if(doubleIt) {
        auto r = double(size.x) / double(size.y);
        if(g.mode == Game::CanvasMode::FreeMode) {
            return { size.x / 10, size.y / 10 };
        } else {
            return { float(g.mode), float(g.mode) / r };
        }
    }
    else return { float(size.x) / g.scaleFactor, float(size.y) / g.scaleFactor };
}

