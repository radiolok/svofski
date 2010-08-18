#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "gpio.h"
#include "leds.h"
#include "LCD-color.h"
#include "Fonts.h"
#include "effector.h"
#include "xprintf.h"

#include "display.h"

#define DISPLAY_TASK_STACK_SIZE     (configMINIMAL_STACK_SIZE * 2)

extern Effector effector;

// Create Display instance here
Display display;

Display::Display()
{
}

void Display::CreateTask(uint32_t priority) {
    queue = xQueueCreate(2, (unsigned portBASE_TYPE) sizeof(uint8_t));
    xTaskCreate(displayTask, (signed char *)"DSPL", DISPLAY_TASK_STACK_SIZE, NULL, priority, (xTaskHandle *)NULL);
}

#define effSize 8
#define checkerSize 16

void Display::drawBackground(int16_t x, int16_t y, int16_t w, int16_t h) {
    for (uint16_t cy = 0; cy < checkerSize*8; cy += checkerSize) {
        for (uint16_t cx = 0; cx < checkerSize*8; cx += checkerSize) {
            if (cx+checkerSize < x || cy+checkerSize < y) continue;
            if (cx > x+w || cy > y+h) continue;
            lcd->FillRect(cx, cy, checkerSize, checkerSize, ((cy&checkerSize)^(cx&checkerSize)) ? colorC1 : colorC2);
        }
    }
}

void Display::drawEffector(uint16_t x, uint16_t y, uint16_t z, bool zap) {
    int left = x - effSize;
    if (left < 0) left = 0;
    int right = x + effSize;
    if (right < 0) 
        right = 0;
    else if (right >= lcd->getWidth()) right = lcd->getWidth();

    int top = y - effSize;
    if (top < 0) top = 0;

    int bottom = y + effSize;
    if (bottom < 0)
        bottom = 0;
    else 
        if (bottom >= lcd->getHeight()) bottom = lcd->getHeight();

    if (zap) {
        drawBackground(left, top, effSize*2+1, effSize*2+1);
    } 
    else {
        lcd->FrameCircle(x, y, z,     zap ? colorBG : colorEff);
        lcd->FrameCircle(x, y, z-1,   zap ? colorBG : colorEff);

        lcd->DrawLine(x, top, x, bottom, zap ? colorBG : colorEff);
        lcd->DrawLine(left, y, right, y, zap ? colorBG : colorEff);
    }
}

void Display::Enqueue(uint8_t cmd, uint8_t param) {
    xQueueSend(queue, &cmd, portMAX_DELAY);
    switch (cmd) {
        case CMD_LCD_CONTRAST:
            xQueueSend(queue, &param, portMAX_DELAY);
            break;
        default:
            break;
    }
}

//volatile char bubuf[7000];

void Display::updateLoop() 
{
    LCD nlcd(&GPIO::lcdCS,
        &GPIO::lcdMOSI,
        &GPIO::lcdCLK,
        &GPIO::lcdRST,
        &g_font3x5, clrTransparent, colorText);

    lcd = &nlcd;

    int16_t px = 0, py = 0, pz = 0;
    char buf[64];

    drawBackground(0, 0, lcd->getWidth(), lcd->getHeight());

    uint32_t ticksBegin = xTaskGetTickCount();
    uint32_t ticksEnd   = ticksBegin; // for benchmark: + 5000;
    bool benchDone = FALSE;

    for (int frames = 0;; frames++) {
        xsprintf(buf, "Eff [%+4d %+4d %+4d]", 
            effector.getX(), 
            effector.getY(),
            effector.getZ());

        // draw the effector
        int16_t x = effector.getX()/2 + lcd->getWidth()/2;
        int16_t y = effector.getZ()/2 + lcd->getHeight()/2;
        int16_t z = 2 + (effector.getY()-180)/16;
        if (z < 0) z = 2;
        if (z > 10) z = 10;

        // clear the area under text
        drawBackground(0,0, lcd->getWidth(), 15);
        // clear the old effector position
        drawEffector(px, py, pz, TRUE);

        // draw new effector position
        drawEffector(x, y, z);
        px = x, py = y, pz = z;

        // draw text
        lcd->Print(buf, 1, FALSE);

        xsprintf(buf, "Ang [%+4d %+4d %+4d]", 
                            effector.getAngle0(),
                            effector.getAngle120(),
                            effector.getAngle240());
        lcd->Print(buf, 2, FALSE);
    
        uint32_t ticksNow = xTaskGetTickCount();
        if (ticksNow > ticksEnd) {
            if (!benchDone) {
                benchDone = TRUE;
                xprintf("%d updates/5 sec", frames);
            }

            // avoid too frequent updates
            vTaskDelay(50/portTICK_RATE_MS);
            blockOnQueue();
        }
    }
}

void Display::blockOnQueue() 
{
    uint8_t qcmd;

    // check the command queue
    if (xQueueReceive(queue, &qcmd, portMAX_DELAY) == pdTRUE) {
        switch (qcmd) {
        case CMD_LCD_CONTRAST: 
                // set contrast
                xQueueReceive(queue, &qcmd, portMAX_DELAY);
                lcd->SetVolume(0, qcmd);
                break;
        case CMD_LCD_INVALIDATE: 
                // invalidate
                for(;qcmd == 1 && xQueuePeek(queue, &qcmd, 0) == pdTRUE;) {
                    if (qcmd == 1) {
                        xQueueReceive(queue, &qcmd, 0); // purge extra invalidate requests
                    }
                }
                break;
        }
    }
}

void Display::displayTask(void *params) { 
    (void)params;

    vTaskDelay(500/portTICK_RATE_MS);

    display.updateLoop();
}
