#include <Image.hpp>
#include <stdexcept>
#include <string>
#include <Game.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_ASSERT(x) { if(!(x)) throw std::runtime_error(std::string("Assert in stb_image: ") + #x); }
#include <stb_image.h>

#if !defined(_WIN32) and !defined(__ANDROID__) && !defined(__IOS__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

using namespace std;
using namespace retro;

static int input_file_read(void* user, char* data, int size) {
    InputFile &file = *((InputFile*) user);
    return file.read(data, size);
}

static void input_file_skip(void* user, int n) {
    InputFile &file = *((InputFile*) user);
    file.seeki(n);
}

static int input_file_eof(void* user) {
    InputFile &file = *((InputFile*) user);
    return file.eof();
}

Image::Image(const string &path, Game &game, Image::Channels desired): game(game), references(*new atomic_size_t(1)) {
    InputFile file = game.openReadFile(path);
    stbi_io_callbacks cbks = {
        input_file_read,
        input_file_skip,
        input_file_eof
    };
    int x, y, c;
    stbi_uc* data = stbi_load_from_callbacks(&cbks, &file, &x, &y, &c, static_cast<int>(desired));
    file.close();
    if(!data) {
        throw runtime_error("Could not load image '" + path + "': " + stbi_failure_reason());
    }

    int depth, pitch;
    if((desired == Image::Undefined && c == STBI_rgb) || desired == Image::RGB) {
        depth = 24;
        pitch = 3 * x;
        this->channels = Image::RGB;
    } else if((desired == Image::Undefined && c == STBI_rgb_alpha) || desired == Image::RGBA) {
        depth = 32;
        pitch = 4 * x;
        this->channels = Image::RGBA;
    } else {
        throw runtime_error("Unsupported pixel format: Grey (with alpha?)");
    }

    SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(data,
                                                 x,
                                                 y,
                                                 depth,
                                                 pitch,
                                                 0x000000ff,
                                                 0x0000ff00,
                                                 0x00ff0000,
                                                 (c == STBI_rgb) ? 0 : 0xff000000);

    this->surface = surf;
    this->data = data;
    this->width = size_t(x);
    this->height = size_t(y);
}

Image::Image(const void* buffer, size_t sizeInBytes, Game &game, Channels desired): game(game), references(*new atomic_size_t(1)) {
    int x, y, c;
    stbi_uc* data = stbi_load_from_memory((const stbi_uc*) buffer, int(sizeInBytes), &x, &y, &c, static_cast<int>(desired));
    if(!data) {
        throw runtime_error("Could not load image from memory: " + string(stbi_failure_reason()));
    }

    int depth, pitch;
    if(c == STBI_rgb) {
        depth = 24;
        pitch = 3 * x;
    } else if(c == STBI_rgb_alpha) {
        depth = 32;
        pitch = 4 * x;
    } else {
        throw runtime_error("Unsupported pixel format: Grey (with alpha?)");
    }

    SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(data,
                                                 x,
                                                 y,
                                                 depth,
                                                 pitch,
                                                 0x000000ff,
                                                 0x0000ff00,
                                                 0x00ff0000,
                                                 (c == STBI_rgb) ? 0 : 0xff000000);

    this->surface = surf;
    this->data = data;
    this->width = size_t(x);
    this->height = size_t(y);
    this->channels = static_cast<Image::Channels>(c);
}

Image::Image(uint32_t* rawData, const glm::uvec2 &size, Channels channels, Game &game): game(game), references(*new atomic_size_t(1)) {
    data = rawData;
    int ch = channels == Image::RGBA ? 4 : 3;
    surface = SDL_CreateRGBSurfaceFrom(data,
                                       size.x,
                                       size.y,
                                       ch*8,
                                       ch*size.x,
                                       0x000000ff,
                                       0x0000ff00,
                                       0x00ff0000,
                                       (channels == Image::RGB) ? 0 : 0xff000000);
    width = size.x;
    height = size.y;
    this->channels = channels;
    doNotFree = true;
}

Image::Image(const Image &image): game(image.game), references(image.references) {
    data = image.data;
    surface = image.surface;
    texture = image.texture;
    width = image.width;
    height = image.height;
    linear = image.linear;
    doNotFree = image.doNotFree;
    references++;
}

Image::Image(Image &&image): game(image.game), references(image.references) {
    data = image.data;
    surface = image.surface;
    texture = image.texture;
    width = image.width;
    height = image.height;
    linear = image.linear;
    doNotFree = image.doNotFree;
    references++;
}

Image::~Image() {
    if(references.fetch_sub(1) == 1) {
        SDL_DestroyTexture((SDL_Texture*) texture);
        SDL_FreeSurface((SDL_Surface*) surface);
        stbi_image_free(data);
        delete &references;
    }
}

void Image::generateAndDestroy() {
    regenerate();
    SDL_FreeSurface((SDL_Surface*) surface);
    stbi_image_free(data);
    surface = data = nullptr;
}

void Image::regenerate() {
    if(texture != nullptr) {
        SDL_DestroyTexture((SDL_Texture*) texture);
    }

    texture = SDL_CreateTextureFromSurface(game.renderer, (SDL_Surface*) surface);
    if(texture == nullptr) {
        throw runtime_error("Could not regenerate texture for image");
    }
}

static inline void render(SDL_Renderer* r, void* tex, SDL_Rect* from, SDL_Rect* to, bool l) {
    if(l) SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_RenderCopy(r, (SDL_Texture*) tex, from, to);
    if(l) SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
}

SDL_Rect get_rekt(const glm::vec2 &pos, const glm::vec2 &size, bool doubleIt);
void Image::draw(const Frame &frame) {
    auto cp = game.currentLevel->ga.camera();
    SDL_Rect rekt = get_rekt(frame.pos - cp, frame.size, game.currentLevel->ga.doubleIt);

    render(game.renderer, texture, NULL, &rekt, linear);
}

void Image::draw(const glm::vec2 &pos) {
    auto cp = game.currentLevel->ga.camera();
    int m = game.currentLevel->ga.doubleIt ? 2 : 1;
    SDL_Rect rekt = {
        static_cast<int>(floor(pos.x - cp.x)) * m,
        static_cast<int>(floor(pos.y - cp.y)) * m,
        static_cast<int>(width) * m,
        static_cast<int>(height) * m
    };

    render(game.renderer, texture, NULL, &rekt, linear);
}

void Image::drawSection(const Frame &section, const Frame &whereToDraw) {
    int m = game.currentLevel->ga.doubleIt ? 2 : 1;
    auto cp = game.currentLevel->ga.camera();
    SDL_Rect rektFrom = {
        static_cast<int>(floor(section.pos.x)),
        static_cast<int>(floor(section.pos.y)),
        static_cast<int>(floor(section.size.x)),
        static_cast<int>(floor(section.size.y))
    }, rektTo = {
        static_cast<int>(floor(whereToDraw.pos.x - cp.x)) * m,
        static_cast<int>(floor(whereToDraw.pos.y - cp.y)) * m,
        static_cast<int>(floor(whereToDraw.size.x)) * m,
        static_cast<int>(floor(whereToDraw.size.y)) * m
    };

    render(game.renderer, texture, &rektFrom, &rektTo, linear);
}

void Image::drawSection(const Frame &section, const glm::vec2 &whereToDraw) {
    int m = game.currentLevel->ga.doubleIt ? 2 : 1;
    auto cp = game.currentLevel->ga.camera();
    SDL_Rect rektFrom = {
        static_cast<int>(floor(section.pos.x)),
        static_cast<int>(floor(section.pos.y)),
        static_cast<int>(floor(section.size.x)),
        static_cast<int>(floor(section.size.y))
    }, rektTo = {
        static_cast<int>(floor(whereToDraw.x - cp.x)) * m,
        static_cast<int>(floor(whereToDraw.y - cp.y)) * m,
        static_cast<int>(floor(section.size.x)) * m,
        static_cast<int>(floor(section.size.y)) * m
    };

    render(game.renderer, texture, &rektFrom, &rektTo, linear);
}

Color Image::pixelAt(size_t x, size_t y) const {
    if(channels == Image::RGBA) {
        auto p = ((const uint8_t*) data) + y * width * 4 + x * 4;
        return {
            p[0],
            p[1],
            p[2],
            p[3]
        };
    } else {
        auto p = ((const uint8_t*) data) + y * width * 3 + x * 3;
        return {
            p[0],
            p[1],
            p[2],
            uint8_t(255)
        };
    }
}

Color& Image::rawPixelAt(size_t x, size_t y) {
    if(channels == Image::RGBA) {
        auto p = ((const uint8_t*) data) + y * width * 4 + x * 4;
        return *((Color*) p);
    } else {
        auto p = ((const uint8_t*) data) + y * width * 3 + x * 3;
        return *((Color*) p);
    }
}

void Image::modifyPixelAt(size_t x, size_t y, Color c) {
    if(channels == Image::RGBA) {
        auto p = ((uint8_t*) data) + y * width * 4 + x * 4;
        p[0] = c.r;
        p[1] = c.g;
        p[2] = c.b;
        p[3] = c.a;
    } else {
        auto p = ((uint8_t*) data) + y * width * 3 + x * 3;
        p[0] = c.r;
        p[1] = c.g;
        p[2] = c.b;
    }
}

Image& Image::operator=(const Image &image) {
    data = image.data;
    surface = image.surface;
    texture = image.texture;
    width = image.width;
    height = image.height;
    linear = image.linear;
    doNotFree = image.doNotFree;
    references++;
    return *this;
}

Image& Image::operator=(Image &&image) {
    data = image.data;
    surface = image.surface;
    texture = image.texture;
    width = image.width;
    height = image.height;
    linear = image.linear;
    doNotFree = image.doNotFree;
    references++;
    return *this;
}

void Image::hdrSetProperties(float gamma, float scale) {
    stbi_hdr_to_ldr_gamma(gamma);
    stbi_hdr_to_ldr_scale(scale);
}
