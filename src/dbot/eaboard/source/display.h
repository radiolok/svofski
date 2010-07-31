#pragma once

#include <inttypes.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "LCD-color.h"

enum {
    CMD_LCD_CONTRAST    = 0,
    CMD_LCD_INVALIDATE  = 1,
};

/// Top-level display driver
class Display {
public:
    Display();
    void CreateTask(uint32_t priority);

    void Enqueue(uint8_t cmd, uint8_t arg = 0);


private:
    void drawBackground(int16_t x, int16_t y, int16_t w, int16_t h);
    void drawEffector(uint16_t x, uint16_t y, uint16_t z, bool zap = FALSE);

    // the actual update loop, never exits, called from the LCD task
    void updateLoop();
    // waits for screen update requests
    void blockOnQueue();

    static void displayTask(void *params);

private:
    LCD* lcd;
    xQueueHandle queue;
};

extern Display display;
