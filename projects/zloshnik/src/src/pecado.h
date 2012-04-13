#pragma once

#include <inttypes.h>

int8_t isin(uint8_t angle);
int8_t icos(uint8_t angle);
void irotate(int16_t* x, int16_t* y, int ox, int oy, uint8_t angle);
