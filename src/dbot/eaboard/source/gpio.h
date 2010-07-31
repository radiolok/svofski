#pragma once

#include "common.h"
#include "InOut.h"

enum {
    JOY_FIRE = 0,
    JOY_LEFT = 1,
    JOY_RIGHT= 3,
    JOY_UP   = 2,
    JOY_DOWN = 4
};

class GPIO {
public:
    static OUT led_red;
    static OUT led_green;
    static OUT led_backlight;
    static OUT buzzer;

    static OUT lcdCS;
    static OUT lcdMOSI;
    static OUT lcdCLK;
    static OUT lcdRST;

    static OUT servo[];

    static IN joy[];
};
