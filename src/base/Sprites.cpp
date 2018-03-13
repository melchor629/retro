#include <Sprites.hpp>
#include <Palette.hpp>
#include <Game.hpp>
#include <Platform.hpp>
#include <memory>
#include <cmath>

#if !defined(_WIN32) and !defined(__ANDROID__) and !defined(__IOS__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

using namespace retro;
using namespace glm;
using namespace std;

Sprites::Sprites(Game &game): game(game), references(*new atomic_size_t(0)) {
    surface = nullptr;
    texture = nullptr;
    pixels  = nullptr;
}

Sprites::Sprites(const string &path, Game &game): game(game), path(path), references(*new atomic_size_t(1)) {
    InputFile i = game.openReadFile(path);
    if(!i.ok()) {
        OutputFile i = game.openWriteFile(path);
        if(!i.ok()) {
            throw runtime_error("Cannot read sprite file '" + path + "'");
        } else {
            this->sprites = 64;
            this->data = reinterpret_cast<uint8_t*>(malloc(this->sprites * 64));
            memset(this->data, 0, this->sprites * 64);
            i.write(reinterpret_cast<char*>(this->data), this->sprites * 64);
            i.write(reinterpret_cast<const char*>(&this->sprites), sizeof(uint64_t));
        }
        i.close();
    } else {
        i.seeki(-sizeof(uint64_t), BasicFile::End);
        i.read((char*) &this->sprites, sizeof(uint64_t));
        i.seeki(0, BasicFile::Beginning);
        this->data = reinterpret_cast<uint8_t*>(malloc(this->sprites * 64));
        i.read(reinterpret_cast<char*>(this->data), this->sprites * 64);
        i.close();
    }

    surface = nullptr;
    texture = nullptr;
    pixels  = nullptr;
}

Sprites::Sprites(const Sprites &other): game(other.game), references(other.references) {
    this->data = other.data;
    this->sprites = other.sprites;
    this->path = other.path;
    this->pixels = other.pixels;
    this->surface = other.surface;
    this->texture = other.texture;

    this->references++;
}

Sprites::Sprites(Sprites &&other): game(other.game), references(other.references) {
    this->data = other.data; other.data = nullptr;
    this->sprites = other.sprites;
    this->path = other.path;
    this->pixels = other.pixels; other.pixels = nullptr;
    this->surface = other.surface; other.surface = nullptr;
    this->texture = other.texture; other.texture = nullptr;

    this->references++;
}

Sprites::~Sprites() {
    if(this->references.fetch_sub(1) == 1) {
        if(texture != nullptr) SDL_DestroyTexture(texture);
        if(surface != nullptr) SDL_FreeSurface(surface);
        if(data != nullptr) free(data);
        if(pixels != nullptr) free(pixels);
        delete &this->references;
    }
}

const Sprite Sprites::operator[](size_t n) {
    return Sprite { n, 8, 8, *this };
}

void Sprites::addSpritesRow() {
    this->sprites += 16;
    this->data = (uint8_t*) realloc(this->data, this->sprites * 8*8);
    memset(data + (sprites - 16) * 8*8, 0, 8*8);
}

void Sprites::save() const {
    OutputFile o = game.openWriteFile(path);
    if(!o.ok()) {
        throw runtime_error("Cannot write sprite file '" + this->path + "'");
    }

    o.write(reinterpret_cast<char*>(this->data), this->sprites * 64);
    o.write(reinterpret_cast<const char*>(&this->sprites), sizeof(uint64_t));
    o.close();
}

void Sprites::reload() {
    InputFile i = game.openReadFile(path);
    i.seeki(-sizeof(uint64_t), BasicFile::End);
    i.read((char*) &this->sprites, sizeof(uint64_t));
    i.seeki(0, BasicFile::Beginning);
    this->data = reinterpret_cast<uint8_t*>(realloc(this->data, this->sprites * 64));
    i.read(reinterpret_cast<char*>(this->data), this->sprites * 64);
    i.close();
}

void Sprites::regenerateTextures() {
    if(texture != nullptr) SDL_DestroyTexture(texture);
    if(surface != nullptr) SDL_FreeSurface(surface);
    size_t w = (8 * 16);
    size_t h = (8 * int(sprites / 16));
    pixels = (uint32_t*) realloc(pixels, w * h * sizeof(uint32_t));

    for(size_t y = 0; y < h; y++) {
        for(size_t x = 0; x < w; x++) {
            uint8_t col = this->data[y * w + x];
            auto rgba = game.getPalette()[size_t(col)].value();
            uint32_t mix = rgba.a << 24 | rgba.b << 16 | rgba.g << 8 | rgba.r;
            this->pixels[y * w + x] = mix;
        }
    }

    surface = SDL_CreateRGBSurfaceWithFormatFrom(this->pixels, w, h, 32, sizeof(uint32_t)*w, SDL_PIXELFORMAT_RGBA32);
    texture = SDL_CreateTextureFromSurface(game.renderer, surface);
}

size_t Sprites::size() const { return this->sprites; };

void Sprites::load(const std::string &path) throw() {
    if(references != 0) throw runtime_error("Cannot load another Sprites file when this instance has already loaded one");
    InputFile i = game.openReadFile(path);
    if(!i.ok()) {
        OutputFile i = game.openWriteFile(path);
        if(!i.ok()) {
            throw runtime_error("Cannot read sprite file '" + path + "'");
        } else {
            this->sprites = 64;
            this->data = reinterpret_cast<uint8_t*>(malloc(this->sprites * 64));
            memset(this->data, 0, this->sprites * 64);
            i.write(reinterpret_cast<char*>(this->data), this->sprites * 64);
            i.write(reinterpret_cast<const char*>(&this->sprites), sizeof(uint64_t));
        }
        i.close();
    } else {
        i.seeki(-sizeof(uint64_t), BasicFile::End);
        i.read(&this->sprites);
        i.seeki(0, BasicFile::Beginning);
        this->data = reinterpret_cast<uint8_t*>(malloc(this->sprites * 64));
        i.read(reinterpret_cast<char*>(this->data), this->sprites * 64);
        i.close();
    }
}

SDL_Renderer* Sprites::renderer() const { return game.renderer; }

Level* Sprites::currentLevel() const {
    return game.currentLevel;
}

Frame Sprites::frameSprite(const Sprite* spr, float &percx, float &percy) const {
    size_t ix = (spr->index % 16) + spr->width / 8 - 1, iy = spr->index + spr->height * 2 - 16;
    //This number is the maximum sprite for the Y axis
    size_t ey = sprites - (16 - (spr->index % 16));
    Frame frame = {
        { (spr->index % 16) * 8, (spr->index / 16) * 8 },
        {
            ix >= 16 ? spr->width - (ix - 15) * 8 : spr->width,
            iy >  ey ? spr->height - (iy - ey) / 16 * 8 : spr->height
        }
    };
    percx = float(frame.size.x) / float(spr->width);
    percy = float(frame.size.y) / float(spr->height);
    return frame;
}

SDL_Rect get_rekt(const vec2 &pos, const vec2 &size, bool doubleIt);
void Sprite::draw(const Frame &frame) const {
    float percx, percy;
    auto frameSpr = this->origin.frameSprite(this, percx, percy);
    auto cp = origin.currentLevel()->ga.camera();
    SDL_Rect src = {
        static_cast<int>(frameSpr.pos.x),
        static_cast<int>(frameSpr.pos.y),
        static_cast<int>(frameSpr.size.x),
        static_cast<int>(frameSpr.size.y)
    };
    SDL_Rect dst = get_rekt(frame.pos - cp, vec2(frame.size.x * 8 * percx, frame.size.y * 8 * percy), origin.currentLevel()->ga.doubleIt);
    SDL_RenderCopy(origin.renderer(), origin.texture, &src, &dst);
}

void Sprite::draw(const vec2 &pos) const {
    float percx, percy;
    auto frameSpr = this->origin.frameSprite(this, percx, percy);
    auto cp = origin.currentLevel()->ga.camera();
    SDL_Rect src = {
        static_cast<int>(frameSpr.pos.x),
        static_cast<int>(frameSpr.pos.y),
        static_cast<int>(frameSpr.size.x),
        static_cast<int>(frameSpr.size.y)
    };
    SDL_Rect dst = get_rekt(pos - cp, vec2(width * percx, height * percy), origin.currentLevel()->ga.doubleIt);
    SDL_RenderCopy(origin.renderer(), origin.texture, &src, &dst);
}

void Sprite::draw(const vec2 &pos, double rotation, int flip) const {
    float percx, percy;
    auto frameSpr = this->origin.frameSprite(this, percx, percy);
    auto cp = origin.currentLevel()->ga.camera();
    SDL_Rect src = {
        static_cast<int>(frameSpr.pos.x),
        static_cast<int>(frameSpr.pos.y),
        static_cast<int>(frameSpr.size.x),
        static_cast<int>(frameSpr.size.y)
    };
    SDL_Rect dst = get_rekt(pos - cp, vec2(width * percx, height * percy), origin.currentLevel()->ga.doubleIt);
    SDL_RenderCopyEx(origin.renderer(), origin.texture, &src, &dst, rotation, nullptr, SDL_RendererFlip(flip));
}

void Sprite::draw(const vec2 &pos, double rotation, const ivec2 &center, int flip) const {
    float percx, percy;
    auto frameSpr = this->origin.frameSprite(this, percx, percy);
    auto cp = origin.currentLevel()->ga.camera();
    SDL_Rect src {
        static_cast<int>(frameSpr.pos.x),
        static_cast<int>(frameSpr.pos.y),
        static_cast<int>(frameSpr.size.x),
        static_cast<int>(frameSpr.size.y)
    };
    SDL_Rect dst = get_rekt(pos - cp, vec2(width * percx, height * percy), origin.currentLevel()->ga.doubleIt);
    SDL_Point ctr { center.x, center.y };
    SDL_RenderCopyEx(origin.renderer(), origin.texture, &src, &dst, rotation, &ctr, SDL_RendererFlip(flip));
}

void Sprite::draw_thicc(const retro::Frame &frame) const {
    float percx, percy;
    auto frameSpr = this->origin.frameSprite(this, percx, percy);
    SDL_Rect src = { static_cast<int>(frameSpr.pos.x), static_cast<int>(frameSpr.pos.y), static_cast<int>(frameSpr.size.x), static_cast<int>(frameSpr.size.y) };
    SDL_Rect dst = get_rekt(frame.pos/* - 2.0f*origin.currentLevel()->ga.camera()*/, frame.size * 8.0f, false);
    SDL_RenderCopy(origin.renderer(), origin.texture, &src, &dst);
}

Frame Sprite::frame() const {
    size_t ix = (index % 16) + width / 8 - 1, iy = index + height * 2 - 16;
    //This number is the maximum sprite for the Y axis
    size_t ey = origin.sprites - (16 - (index % 16));
    return {
        { (index % 16) * 8, (index / 16) * 8 },
        {
            ix >= 16 ? width - (ix - 15) * 8 : width,
            iy >  ey ? height - (iy - ey) / 16 * 8 : height
        }
    };
}
