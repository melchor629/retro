#include <Map.hpp>
#include <Platform.hpp>
#include <glm/vec4.hpp>
#include <Game.hpp>
#include <Sprites.hpp>

#if !defined(_WIN32) and !defined(__ANDROID__) and !defined(__IOS__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

using namespace retro;
using namespace glm;
using namespace std;

Map Map::createMap(const string &path, Game &g, const Sprites &sprites, const uvec2 &initialSize) {
    OutputFile i = g.openWriteFile(path);
    for(size_t p = 0; p < initialSize.x * initialSize.y; p++) {
        char c = '\0';
        i.write(&c, sizeof(char));
    }
    i.write(sprites.path.c_str(), sprites.path.length());
    i << '\n';
    i.write<uint32_t>(&initialSize.x, 1);
    i.write<uint32_t>(&initialSize.y, 1);
    i.close();
    return Map(path, g);
}

Map::Map(const string &path, Game &g): game(g), data(*new uint8_t*(nullptr)), path(path), references(*new atomic_size_t(1)) {
    InputOutputFile i = g.openFile(path);
    if(!i.ok()) {
        throw runtime_error("Cannot read map file '" + path + "'");
    } else {
        if(i.seeki(-int64_t(2ull*sizeof(size.x)), BasicFile::End) == -1)
            throw runtime_error("Invalid map file '" + path + "'");
        i.read<uint32_t>(&size.x, 1);
        i.read<uint32_t>(&size.y, 1);
        i.seeki(0, BasicFile::Beginning);
        this->data = reinterpret_cast<uint8_t*>(malloc(size.x * size.y));
        i.read(reinterpret_cast<char*>(data), size.x * size.y);
        string spritePath = i.readline();
        sprites = new Sprites(spritePath, g);
        i.close();
    }

    surface = nullptr;
    texture = nullptr;
    pixels  = nullptr;
}

Map::Map(const Map &map): game(map.game), data(map.data), sprites(map.sprites), references(map.references) {
    size = map.size;
    path = map.path;
    pixels = map.pixels;
    surface = map.surface;
    texture = map.texture;
    references++;
}

Map::Map(Map &&map): game(map.game), data(map.data), sprites(map.sprites), references(map.references) {
    size = map.size;
    path = map.path;
    pixels = map.pixels; map.pixels = nullptr;
    surface = map.surface; map.surface = nullptr;
    texture = map.texture; map.texture = nullptr;
    references++;
}

Map::~Map() {
    if(references.fetch_sub(1) == 1) {
        if(sprites != nullptr) delete sprites;
        if(texture != nullptr) SDL_DestroyTexture(texture);
        if(texture != nullptr) SDL_FreeSurface(surface);
        if(pixels  != nullptr) free(pixels);
        if(data    != nullptr) free(data);
        delete &data;
        delete &references;
    }
}

uint8_t& Map::at(size_t x, size_t y) {
    return data[y * size.x + x];
}

void Map::resize(const uvec2 &size) {
    //TODO
}

void Map::regenerateTextures() {
    if(texture != nullptr) SDL_DestroyTexture(texture);
    if(surface != nullptr) SDL_FreeSurface(surface);
    uint32_t w = 8 * size.x;
    uint32_t h = 8 * size.y;
    pixels = (uint32_t*) realloc(pixels, w * h * sizeof(uint32_t));

    for(size_t y = 0; y < h; y++) {
        for(size_t x = 0; x < w; x++) {
            uint8_t nsprite = data[y / 8 * w / 8 + x / 8];
            if(nsprite > 0) {
                const Sprite sprite = (*sprites)[nsprite - 1];
                uint8_t col = sprite.at(x % 8, y % 8);
                if(col != 0) {
                    Color rgba = game.getPalette()[size_t(col)].value();
                    uint32_t mix = rgba.a << 24 | rgba.b << 16 | rgba.g << 8 | rgba.r;
                    pixels[y * w + x] = mix;
                } else {
                    pixels[y * w + x] = 0x00000000;
                }
            } else {
                pixels[y * w + x] = 0x00000000;
            }
        }
    }

    surface = SDL_CreateRGBSurfaceFrom(pixels, w, h, 32, sizeof(uint32_t)*w, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    texture = SDL_CreateTextureFromSurface(game.renderer, surface);

    sprites->regenerateTextures();
}

void Map::save() {
    OutputFile o = game.openWriteFile(path);
    if(!o.ok()) {
        throw runtime_error("Cannot write map file '" + this->path + "'");
    }

    o.write(reinterpret_cast<char*>(data), size.x * size.y);
    o.write(sprites->path.c_str(), sprites->path.length());
    o << '\n';
    o.write(reinterpret_cast<const char*>(&size.x), sizeof(size.x));
    o.write(reinterpret_cast<const char*>(&size.y), sizeof(size.y));
    o.close();
}

void Map::reload() {
    InputFile i = game.openReadFile(path);
    uvec2 fileSize;
    i.seeki(-int64_t(2ull * sizeof(size.x)), BasicFile::End);
    i.read<uint32_t>(&fileSize.x, 1);
    i.read<uint32_t>(&fileSize.y, 1);
    i.seeki(0, BasicFile::Beginning);
    i.read(reinterpret_cast<char*>(data), size.x * size.y);
    i.close();
	sprites->reload();
}

SDL_Rect get_rekt(const vec2 &pos, const vec2 &size, bool doubleIt);
void Map::draw(const Frame &frame) {
    float px = glm::min(glm::max(-(frame.pos.x), 0.0f), (size.x * 8.0f));
    float py = glm::min(glm::max(-(frame.pos.y), 0.0f), (size.y * 8.0f));
    float px2 = glm::max((frame.pos.x), 0.0f);
    float py2 = glm::max((frame.pos.y), 0.0f);
    auto cp = game.currentLevel->ga.camera();
    SDL_Rect src = {
        static_cast<int>(px),
        static_cast<int>(py),
        static_cast<int>(frame.size.x),
        static_cast<int>(frame.size.y)
    };
    SDL_Rect dst = get_rekt(vec2(px2, py2) - cp, frame.size, game.currentLevel->ga.doubleIt);
    SDL_RenderCopy(game.renderer, texture, &src, &dst);
}

const Sprites* Map::getSprites() const {
    return sprites;
}
