#pragma once

#include "InOut.h"
#include "common.h"

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
};
