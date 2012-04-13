#pragma once

#include <inttypes.h>

class Text {
public:
    void Char(uint8_t c, int ox, int oy, uint8_t scale);
    void Str(const char *s, uint8_t scale, uint8_t angle);
};

extern Text text;
