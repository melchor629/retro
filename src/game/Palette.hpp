#pragma once

#include <Palette.hpp>

class PaletteTest: public retro::Palette {

    virtual retro::Optional<Color> getColour(size_t idx) const override {
        switch(idx) {
            //White-Grey-Black
            case   0: return 0x000000FF_rgba;
            case   1: return 0x242424FF_rgba;
            case   2: return 0x494949FF_rgba;
            case   3: return 0x6D6D6DFF_rgba;
            case   4: return 0x929292FF_rgba;
            case   5: return 0xB6B6B6FF_rgba;
            case   6: return 0xDBDBDBFF_rgba;
            case   7: return 0xFFFFFFFF_rgba;

            //Yellow
            case   8: return 0x403E03FF_rgba;
            case   9: return 0x7F7C05FF_rgba;
            case  10: return 0xBFBA08FF_rgba;
            case  11: return 0xDED80AFF_rgba;
            case  12: return 0xFFF80BFF_rgba;
            case  13: return 0xFFF969FF_rgba;
            case  14: return 0xFFFBABFF_rgba;
            case  15: return 0xFFFAD0FF_rgba;

            //Green
            case  16: return 0x0A5E05FF_rgba;
            case  17: return 0x109E09FF_rgba;
            case  18: return 0x14C40BFF_rgba;
            case  19: return 0x17DE0DFF_rgba;
            case  20: return 0x18EB0DFF_rgba;
            case  21: return 0x77DE68FF_rgba;
            case  22: return 0xA3DE9BFF_rgba;
            case  23: return 0xBBDEB8FF_rgba;

            //Cyan
            case  24: return 0x005E5CFF_rgba;
            case  25: return 0x007D7AFF_rgba;
            case  26: return 0x009E9AFF_rgba;
            case  27: return 0x00C4C0FF_rgba;
            case  28: return 0x00DED9FF_rgba;
            case  29: return 0x72EBE5FF_rgba;
            case  30: return 0x8CDED9FF_rgba;
            case  31: return 0xABDED9FF_rgba;

            //Blue
            case  32: return 0x081140FF_rgba;
            case  33: return 0x0E163DFF_rgba;
            case  34: return 0x1D2D7DFF_rgba;
            case  35: return 0x263AA3FF_rgba;
            case  36: return 0x2C44BDFF_rgba;
            case  37: return 0x4A5DBDFF_rgba;
            case  38: return 0x7685CCFF_rgba;
            case  39: return 0xA5ADD9FF_rgba;

            //Violet
            case  40: return 0x2E094CFF_rgba;
            case  41: return 0x54108CFF_rgba;
            case  42: return 0x6B14B2FF_rgba;
            case  43: return 0x7B17CCFF_rgba;
            case  44: return 0x9029E5FF_rgba;
            case  45: return 0xAB70DBFF_rgba;
            case  46: return 0xCEA5F0FF_rgba;
            case  47: return 0xDBBFF2FF_rgba;

            //Pink
            case  48: return 0x630A66FF_rgba;
            case  49: return 0x830787FF_rgba;
            case  50: return 0xA011A6FF_rgba;
            case  51: return 0xDE17E5FF_rgba;
            case  52: return 0xE251E8FF_rgba;
            case  53: return 0xE675EBFF_rgba;
            case  54: return 0xEC99F0FF_rgba;
            case  55: return 0xF3C4F5FF_rgba;

            //Magenta
            case  56: return 0x400523FF_rgba;
            case  57: return 0x7F0A47FF_rgba;
            case  58: return 0xBF0F6AFF_rgba;
            case  59: return 0xFF138EFF_rgba;
            case  60: return 0xFA3C9FFF_rgba;
            case  61: return 0xFA69B4FF_rgba;
            case  62: return 0xF78BC3FF_rgba;
            case  63: return 0xFFBADEFF_rgba;

            //Red
            case  64: return 0x361010FF_rgba;
            case  65: return 0x631616FF_rgba;
            case  66: return 0x9E3131FF_rgba;
            case  67: return 0xB53837FF_rgba;
            case  68: return 0xF54B4BFF_rgba;
            case  69: return 0xF55D5DFF_rgba;
            case  70: return 0xFF8585FF_rgba;
            case  71: return 0xFFA3A3FF_rgba;

            //Orang
            case  72: return 0x6B2A0CFF_rgba;
            case  73: return 0x91330AFF_rgba;
            case  74: return 0xAB4214FF_rgba;
            case  75: return 0xD15118FF_rgba;
            case  76: return 0xED5C1CFF_rgba;
            case  77: return 0xF57740FF_rgba;
            case  78: return 0xFC9B6FFF_rgba;
            case  79: return 0xFFBA9CFF_rgba;
            default: return {};
        }
    }

    virtual retro::Optional<Color> getColourByName(const char* colorName) const override {
        if(colorName == nullptr) return {};
        string colorNameGut = colorName;
        if(colorNameGut == "white") return (*this)[7];
        if(colorNameGut == "grey") return (*this)[3];
        if(colorNameGut == "gray") return (*this)[3];
        if(colorNameGut == "black") return (*this)[1];
        if(colorNameGut == "yellow") return (*this)[12];
        if(colorNameGut == "green") return (*this)[20];
        if(colorNameGut == "cyan") return (*this)[28];
        if(colorNameGut == "blue") return (*this)[36];
        if(colorNameGut == "violet") return (*this)[43];
        if(colorNameGut == "pink") return (*this)[51];
        if(colorNameGut == "red") return (*this)[68];
        if(colorNameGut == "orange") return (*this)[76];
        return {};
    }

public:

    virtual size_t size() const override { return 80; }

};

