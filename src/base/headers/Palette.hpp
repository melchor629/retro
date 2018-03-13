#pragma once

#include <glm/vec4.hpp>
#include <Platform.hpp>
#include <cstring>
#include <Color.hpp>
#include "Optional.hpp"

namespace retro {

    /// A palette of colours.
    class Palette {
    protected:

        /// A colour from your implementation. From 1 to size - 1.
        virtual Optional<Color> getColour(size_t idx) const = 0;
        /// A colour from your implementation, by name.
        virtual Optional<Color> getColourByName(const char*) const = 0;

    public:

        /// Gets a colour by index, if exists.
        Optional<Color> operator[](size_t idx) const {
            if(idx == 0) return 0_rgba;
            else return getColour(idx);
        }

        /// Gets a colour by name, if exists.
        Optional<Color> operator[](const char* name) const {
            if(!strcmp("transparent", name) || !strcmp("default", name)) {
                return 0_rgba;
            } else return getColourByName(name);
        }

        /// Gets the number of colours in total.
        virtual size_t size() const { return 1; }
        /// Gets the number of colours in total.
        inline size_t length() const { return size(); }
        virtual ~Palette() {};

    };

    /// Implementation of Palette that reads a Gimp palette file.
    class GimpPalette: public Palette {

        Optional<Color> colors[256];
        size_t len;

    protected:

        virtual Optional<Color> getColour(size_t idx) const override {
            return this->colors[idx];
        }

        virtual Optional<Color> getColourByName(const char*) const override {
            return {};
        }

    public:

        /// Reads from an input stream and creates the Palette from it.
        GimpPalette(InputFile &i) {
            i.seeki(13);
            while(i.get() != '\n');
            while(i.get() != '\n');
            size_t pos = 0;
            while(!i.eof()) {
                uint32_t r, g, b, a;
                i >> r >> g >> b >> a;
                if(a == pos) {
                    this->colors[pos] = rgba(r, g, b, 255);
                }
                pos++;
            }
            this->len = pos - 1;
        }

        virtual size_t size() const override { return len; }

    };

    static inline Color HSBtoRGB(const glm::vec4 &hsb);

    /// Implementation of Palette that reads a Photoshop palette file.
    class PhotoshopPalette: public Palette {

        Optional<Color> colors[256];
        size_t len;

        inline uint16_t rword(InputFile &i) {
            uint8_t a, b;
            i.read<uint8_t>(&a, 1);
            i.read<uint8_t>(&b, 1);
            return uint16_t(uint16_t(a) << 8) | b;
        }

        inline Optional<Color> readColor(InputFile &i, uint16_t ver) {
            uint16_t colorSpace = rword(i);
            Optional<Color> color;
            if(colorSpace == 0) {
                color = Color {
                    rword(i) / 256,
                    rword(i) / 256,
                    rword(i) / 256,
                    0xFF
                };
                rword(i);
            } else if(colorSpace == 1) {
                color = HSBtoRGB({
                    rword(i) / 182.04f,
                    rword(i) / 655.35f,
                    rword(i) / 655.35f,
                    0xFF
                });
                rword(i);
            }

            if(ver == 2) {
                rword(i);
                uint16_t skip = rword(i);
                i.seeki(skip);
                rword(i);
            }

            return color;
        }

    protected:

        virtual Optional<Color> getColour(size_t idx) const override {
            return this->colors[idx];
        }

        virtual Optional<Color> getColourByName(const char*) const override {
            return {};
        }

    public:

        /// Reads from an input stream and creates a Palette from it.
        PhotoshopPalette(InputFile &i) {
            uint16_t ver = rword(i);
            this->len = rword(i);
            for(size_t pos = 0; pos < this->len; pos++) {
                this->colors[pos] = readColor(i, ver);
            }
        }

        virtual size_t size() const override { return len; }

    };

    /// Writes a palette to a Gimp palette file.
    static inline void writeToGPL(const Palette &p, std::ostream &o, const char* name = "RetroPalette") {
        o << "GIMP Palette\n";
        o << "Name: " << name << '\n';
        o << "#\n";
        for(size_t i = 0; i < p.size(); i++) {
            auto col = p[i];
            if(col) {
                o << col->r << ' ' << col->g << ' ' << col->b << ' ' << i << '\n';
            } else {
                o << "0 0 0 " << i << "-undefined\n";
            }
        }
    }

    // http://websemantics.github.io/Color-Palette-Toolkit/
    /// Writes a palette to a Photoshop palette file.
    static inline void writeToACO(const Palette &p, std::ostream &o) {
        o << '\0' << '\1' << uint8_t(p.size() >> 8) << uint8_t(p.size() & 0xFF);
        for(size_t i = 0; i < p.size(); i++) {
            auto col = p[i];
            if(col) {
                uint16_t r = col->r * 256;
                uint16_t g = col->g * 256;
                uint16_t b = col->b * 256;
                o << '\0' << '\0';
                o << uint8_t(r >> 8) << uint8_t(r & 0xFF);
                o << uint8_t(g >> 8) << uint8_t(g & 0xFF);
                o << uint8_t(b >> 8) << uint8_t(b & 0xFF);
                o << '\0' << '\0';
            }
        }
    }

    /// Converts from HSB to RGB
    static inline Color HSBtoRGB(const glm::vec4 &hsb) {
        Color color;
        if(hsb.g == 0) { //Saturation
            color.r = color.g = color.b = uint8_t(hsb.b * 255.0f + 0.5f);
        } else {
            int h = int(hsb.r - float(int(hsb.r))) * 6;
            float f = h - float(int(h));
            float p = hsb.b * (1.0f - hsb.g);
            float q = hsb.b * (1.0f - hsb.g * f);
            float t = hsb.b * (1.0f - hsb.g * (1.0f - f));
            switch(h) {
                case 0:
                    color.r = (int) (hsb.b * 255.0f + 0.5f);
                    color.g = (int) (t * 255.0f + 0.5f);
                    color.b = (int) (p * 255.0f + 0.5f);
                    break;
                case 1:
                    color.r = (int) (q * 255.0f + 0.5f);
                    color.g = (int) (hsb.b * 255.0f + 0.5f);
                    color.b = (int) (p * 255.0f + 0.5f);
                    break;
                case 2:
                    color.r = (int) (p * 255.0f + 0.5f);
                    color.g = (int) (hsb.b * 255.0f + 0.5f);
                    color.b = (int) (t * 255.0f + 0.5f);
                    break;
                case 3:
                    color.r = (int) (p * 255.0f + 0.5f);
                    color.g = (int) (q * 255.0f + 0.5f);
                    color.b = (int) (hsb.b * 255.0f + 0.5f);
                    break;
                case 4:
                    color.r = (int) (t * 255.0f + 0.5f);
                    color.g = (int) (p * 255.0f + 0.5f);
                    color.b = (int) (hsb.b * 255.0f + 0.5f);
                    break;
                case 5:
                    color.r = (int) (hsb.b * 255.0f + 0.5f);
                    color.g = (int) (p * 255.0f + 0.5f);
                    color.b = (int) (q * 255.0f + 0.5f);
                    break;
            }
        }
        color.a = uint8_t(hsb.a * 256.0f);
        return color;
    }

    /// Converts from RGB to HSB
    static inline glm::vec4 RGBtoHSB(const Color rgb) {
        float hue, saturation, brightness;
        unsigned cmax = (rgb.r > rgb.g) ? rgb.r : rgb.g;
        if(rgb.b > cmax) cmax = rgb.b;
        unsigned cmin = (rgb.r < rgb.g) ? rgb.r : rgb.g;
        if(rgb.b < cmin) cmin = rgb.b;

        brightness = ((float) cmax) / 255.0f;
        if(cmax != 0)
            saturation = ((float) (cmax - cmin)) / ((float) cmax);
        else
            saturation = 0;
        if(saturation == 0)
            hue = 0;
        else {
            float redc = ((float) (cmax - rgb.r)) / ((float) (cmax - cmin));
            float greenc = ((float) (cmax - rgb.g)) / ((float) (cmax - cmin));
            float bluec = ((float) (cmax - rgb.b)) / ((float) (cmax - cmin));
            if (rgb.r == cmax)
                hue = bluec - greenc;
            else if (rgb.g == cmax)
                hue = 2.0f + redc - bluec;
            else
                hue = 4.0f + greenc - redc;
            hue = hue / 6.0f;
            if(hue < 0)
                hue = hue + 1.0f;
        }

        return { hue, saturation, brightness, rgb.a / 256.0f };
    }

}
