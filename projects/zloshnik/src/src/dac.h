#pragma once

#include <inttypes.h>

class Dac {
public:
    void Setup();
    void SetXY(uint8_t dacA, uint8_t dacB);
    void SetZ(uint8_t dacZ);
};

extern Dac xyz;

