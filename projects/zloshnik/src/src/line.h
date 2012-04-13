#pragma once

#include <inttypes.h>

extern int current_x, current_y;

void move_to(int x, int y);
void line_to(int x1, int y1);
void clamp(int* x);
