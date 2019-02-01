#pragma once

#include <cstdint>
#include <string>

class Text
{
public:
    static const int8_t FG_BLACK = 30;
    static const int8_t FG_RED = 31;
    static const int8_t FG_GREEN = 32;
    static const int8_t FG_YELLOW = 33;
    static const int8_t FG_BLUE = 34;
    static const int8_t FG_MAGENTA = 35;
    static const int8_t FG_CYAN = 36;
    static const int8_t FG_WHITE = 37;

    static const int8_t FG_DEFAULT = 37;
    static const int8_t BG_BLACK = 40;
    static const int8_t BG_RED = 41;
    static const int8_t BG_GREEN = 42;
    static const int8_t BG_YELLOW = 43;
    static const int8_t BG_BLUE = 44;
    static const int8_t BG_MAGENTA = 45;
    static const int8_t BG_CYAN = 46;
    static const int8_t BG_WHITE = 47;

    static const int8_t BG_DEFAULT = 49;
    static const int8_t BOLD = 1;
    static const int8_t UNDERLINE = 4;

    static const int8_t INVERSE = 7;
    static const int8_t RESET_ALL = 0;
    static const int8_t RESET_BOLD = 21;
    static const int8_t RESET_UNDERLINE = 24;
    static const int8_t RESET_INVERSE = 27;

    static const char HORIZONTAL_LINE_CHAR{'-'};

    static std::string decor(int8_t code = RESET_ALL);
    static std::string hline(uint16_t width = 50, char lineChar = HORIZONTAL_LINE_CHAR);
    static std::string justifyL(std::string string, uint16_t width, char fillChar = ' ');
    static std::string justifyR(std::string string, uint16_t width, char fillChar = ' ');
};


