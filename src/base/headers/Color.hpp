#pragma once

#include <type_traits>
#include <functional>
#include <stdint.h>
#include <glm/vec4.hpp>

namespace retro {

    /// Represents a color using unsigned ints
    /**
     * The Color structure allows you to represent RGB or RGBA colours
     * using the constructor Color() or the operator _rgb() / _rgba().
     * You can compare colours, sum them, substract them, multiply or
     * divide by a float. And if you prefer Colour instead of Color...
     */
    struct Color {
        typedef uint8_t vtype;
        vtype r, g, b, a;

        constexpr Color(): Color(0, 0, 0, 0) {}
        template<typename T, typename = std::enable_if<std::is_integral<T>::value>>
        constexpr Color(T r, T g, T b, T a):
        r(vtype(r)), g(vtype(g)), b(vtype(b)), a(vtype(a)) {}
        constexpr Color(const Color &c): r(c.r), g(c.g), b(c.b), a(c.a) {}

        constexpr bool operator==(const Color &o) const {
            return r == o.r && g == o.g && b == o.b && a == o.a;
        }

        constexpr bool operator!=(const Color &o) const {
            return !(*this == o);
        }

        Color& operator=(const Color &o) {
            r = o.r;
            g = o.g;
            b = o.b;
            a = o.a;
            return *this;
        }

        Color& operator=(Color &&o) {
            r = o.r;
            g = o.g;
            b = o.b;
            a = o.a;
            return *this;
        }
    };

    typedef Color Colour;

    constexpr Color operator+(const Color &a, const Color &b) {
        return {
            static_cast<Color::vtype>(a.r + b.r),
            static_cast<Color::vtype>(a.g + b.g),
            static_cast<Color::vtype>(a.b + b.b),
            static_cast<Color::vtype>(a.a + b.a)
        };
    }

    constexpr Color operator-(const Color &a, const Color &b) {
        return {
            static_cast<Color::vtype>(a.r - b.r),
            static_cast<Color::vtype>(a.g - b.g),
            static_cast<Color::vtype>(a.b - b.b),
            static_cast<Color::vtype>(a.a - b.a)
        };
    }

    constexpr Color operator*(const Color &a, long double d) {
        return {
            static_cast<Color::vtype>(a.r * d),
            static_cast<Color::vtype>(a.g * d),
            static_cast<Color::vtype>(a.b * d),
            static_cast<Color::vtype>(a.a * d)
        };
    }

    constexpr Color operator/(const Color &a, long double d) {
        return {
            static_cast<Color::vtype>(a.r / d),
            static_cast<Color::vtype>(a.g / d),
            static_cast<Color::vtype>(a.b / d),
            static_cast<Color::vtype>(a.a / d)
        };
    }

    constexpr Color operator*(const Color &a, const glm::vec4 &vec) {
        return {
            static_cast<Color::vtype>(a.r * vec.r),
            static_cast<Color::vtype>(a.g * vec.g),
            static_cast<Color::vtype>(a.b * vec.b),
            static_cast<Color::vtype>(a.a * vec.a)
        };
    }

    constexpr Color operator/(const Color &a, const glm::vec4 &vec) {
        return {
            static_cast<Color::vtype>(a.r / vec.r),
            static_cast<Color::vtype>(a.g / vec.g),
            static_cast<Color::vtype>(a.b / vec.b),
            static_cast<Color::vtype>(a.a / vec.a)
        };
    }

    /// Allows you to write a hex color like that: `Color c = 0xRRGGBBAA_rgba;`
    constexpr Color operator"" _rgba(unsigned long long col) {
        return {
            static_cast<Color::vtype>((col >> 24) & 0xFF),
            static_cast<Color::vtype>((col >> 16) & 0xFF),
            static_cast<Color::vtype>((col >> 8) & 0xFF),
            static_cast<Color::vtype>(col & 0xFF)
        };
    }

    /// Allows you to write a hex color like that: `Color c = 0xRRGGBB_rgb;`
    constexpr Color operator"" _rgb(unsigned long long col) {
        return {
            static_cast<Color::vtype>((col >> 16) & 0xFF),
            static_cast<Color::vtype>((col >> 8) & 0xFF),
            static_cast<Color::vtype>(col & 0xFF),
            Color::vtype(0xFF)
        };
    }

    constexpr Color::vtype charToNum(char c) {
        if('0' <= c && c <= '9') {
            return c - '0';
        } else if('a' <= c && c <= 'f') {
            return c - 'a' + 10;
        } else if('A' <= c && c <= 'F') {
            return c - 'A' + 10;
        } else {
            return 0;
        }
    }

    /// Allows you to write a hex color like that: `Color c = "#RRGGBBAA"_rgba;`
    constexpr Color operator"" _rgba(const char* c, size_t s) {
        return s >= 9 ? Color{
            static_cast<Color::vtype>(charToNum(c[1]) << 4 | charToNum(c[2])),
            static_cast<Color::vtype>(charToNum(c[3]) << 4 | charToNum(c[4])),
            static_cast<Color::vtype>(charToNum(c[5]) << 4 | charToNum(c[6])),
            static_cast<Color::vtype>(charToNum(c[7]) << 4 | charToNum(c[8]))
        } : Color{ 0, 0, 0, 0 };
    }

    /// Allows you to write a hex color like that: `Color c = "#RRGGBB"_rgb;`
    constexpr Color operator"" _rgb(const char* c, size_t s) {
        return s >= 7 ? Color{
            static_cast<Color::vtype>(charToNum(c[1]) << 4 | charToNum(c[2])),
            static_cast<Color::vtype>(charToNum(c[3]) << 4 | charToNum(c[4])),
            static_cast<Color::vtype>(charToNum(c[5]) << 4 | charToNum(c[6])),
            Color::vtype(0xFF)
        } : Color{ 0, 0, 0, 0 };
    }

    /// Creates a Color using this values, its like CSS
    constexpr Color rgba(uint32_t r, uint32_t g, uint32_t b, uint32_t a) {
        return { Color::vtype(r), Color::vtype(g), Color::vtype(b), Color::vtype(a) };
    }

    /// Creates a Color using this values, its like CSS
    constexpr Color rgb(uint32_t r, uint32_t g, uint32_t b) {
        return { Color::vtype(r), Color::vtype(g), Color::vtype(b), Color::vtype(0xFF) };
    }

}

namespace std {
    template<>
    struct hash<retro::Color> {
        size_t operator()(const retro::Color &vec) const {
            return ((vec.r & 0xFF) << 24) | ((vec.g & 0xFF) << 16) | ((vec.b & 0xFF) << 8) | (vec.a & 0xFF);
        }
    };
}
