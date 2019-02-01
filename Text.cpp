#include <sstream>
#include <iomanip>

#include "Text.h"

std::string Text::decor(int8_t code)
{
    return "\33[" + std::to_string(static_cast<int>(code)) + "m";
}

std::string Text::hline(uint16_t width, char lineChar)
{
    return std::string(width, lineChar);
}

std::string Text::justifyL(std::string string, uint16_t width, char fillChar)
{
    std::stringstream ss;
    ss << std::setw(width);
    ss << std::setfill(fillChar);
    ss << std::left;

    ss << string;

    return ss.str();
}

std::string Text::justifyR(std::string string, uint16_t width, char fillChar)
{
    std::stringstream ss;
    ss << std::setw(width);
    ss << std::setfill(fillChar);
    ss << std::right;

    ss << string;

    return ss.str();
}
