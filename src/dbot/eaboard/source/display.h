#pragma once

#include <inttypes.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "LCD-color.h"

/// Commands for the display handler.
/// See Display::Enqueue()
enum CMD_LCD {
    CMD_LCD_CONTRAST    = 0,    ///< Set contrast, the parameter is contrast
    CMD_LCD_INVALIDATE  = 1,    ///< Refresh display
};

/// \brief Top-level display module
///
/// Represents the display update task that
/// runs infinitely, waiting on the queue to
/// update.
///
/// Enqueue() puts a command to the display queue,
/// see CMD_LCD_CONTRAST and CMD_LCD_INVALIDATE.
///
/// The update method actually draws everything on the LCD.
/// LCD driver class is constructed here and no accesses
/// to it should be done from anywhere outside of the
/// display update loop.
///
class Display {
public:
    Display();
    void CreateTask(uint32_t priority);

    /// Enqueue a display command. See enum ::CMD_LCD
    void Enqueue(uint8_t cmd, uint8_t arg = 0);


private:
    void drawBackground(int16_t x, int16_t y, int16_t w, int16_t h);
    void drawEffector(uint16_t x, uint16_t y, uint16_t z, bool zap = FALSE);

    /// The actual update loop, never exits, called from displayTask().
    /// The LCD driver instance is created here.
    void updateLoop();

    /// waits indefinitely for screen update requests
    void blockOnQueue();

    /// the task wrapper, passes control to instance's updateLoop()
    static void displayTask(void *params);

private:
    LCD* lcd;
    xQueueHandle queue;
};

extern Display display;
