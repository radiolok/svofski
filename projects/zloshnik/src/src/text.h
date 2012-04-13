#pragma once

#include <inttypes.h>

#define TEXT_CHARWIDTH 6
#define TEXT_CHARHEIGHT 9

class Text {
private:
    uint8_t text_angle;
public:
    void Char(uint8_t c, int ox, int oy, uint8_t scale);
    void Str(const char *s, uint8_t scale, uint8_t angle);
};

extern Text text;
