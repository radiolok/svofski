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

extern Effector effector;

static portTASK_FUNCTION_PROTO( ledFlashTask, pvParameters );
static portTASK_FUNCTION_PROTO( lcdControlTask, pvParameters);

static int nled = 0;

xQueueHandle qhLCD;

void startLEDFlashTasks(unsigned portBASE_TYPE uxPriority) {
    int i;

    for (i = 0; i < 2; i++) {
        xTaskCreate(ledFlashTask, (signed char *) "LEDx", configMINIMAL_STACK_SIZE*2, NULL, uxPriority, (xTaskHandle *)NULL);
    }

    xTaskCreate(lcdControlTask, (signed char *)"LCD", configMINIMAL_STACK_SIZE, NULL, uxPriority, (xTaskHandle *)NULL);

    qhLCD = xQueueCreate(2, (unsigned portBASE_TYPE) sizeof(char));
}

static portTASK_FUNCTION( ledFlashTask, pvParameters )
{
    portTickType xFlashRate, xLastFlashTime;

    int led_id = nled++;

    /* The parameters are not used. */
    ( void ) pvParameters;

    xFlashRate = (250 + 300*led_id) / portTICK_RATE_MS; // 250ms on/250 ms off

    xLastFlashTime = xTaskGetTickCount();
    for (;;) {
        (led_id ? &GPIO::led_red : &GPIO::led_green)->On();
        vTaskDelayUntil(&xLastFlashTime, 100/portTICK_RATE_MS);
        (led_id ? &GPIO::led_red : &GPIO::led_green)->Off();
        vTaskDelayUntil(&xLastFlashTime, xFlashRate);
    }
}

#define effSize 8
#define checkerSize 16

static void drawBackground(LCD* lcd, int16_t x, int16_t y, int16_t w, int16_t h) {
    for (uint16_t cy = 0; cy < checkerSize*8; cy += checkerSize) {
        for (uint16_t cx = 0; cx < checkerSize*8; cx += checkerSize) {
            if (cx+checkerSize < x || cy+checkerSize < y) continue;
            if (cx > x+w || cy > y+h) continue;
            lcd->FillRect(cx, cy, checkerSize, checkerSize, ((cy&checkerSize)^(cx&checkerSize)) ? colorC1 : colorC2);
        }
    }
}

static void drawEffector(LCD* lcd, uint16_t x, uint16_t y, uint16_t z, bool zap = FALSE) {
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
        //lcd->FillRect(left, top, effSize*2+1, effSize*2+1, colorBG);
        drawBackground(lcd, left, top, effSize*2+1, effSize*2+1);
    } 
    else {
        lcd->FrameCircle(x, y, z,     zap ? colorBG : colorEff);
        lcd->FrameCircle(x, y, z-1,     zap ? colorBG : colorEff);
        lcd->DrawLine(x, top, x, bottom, zap ? colorBG : colorEff);
        lcd->DrawLine(left, y, right, y, zap ? colorBG : colorEff);
    }
}

static portTASK_FUNCTION(lcdControlTask, pvParameters) { (void)pvParameters;
    char buf[64];
    int16_t px, py, pz;

    uint8_t qcmd;

    vTaskDelay(500/portTICK_RATE_MS);

    LCD lcd(&GPIO::lcdCS,
           &GPIO::lcdMOSI,
           &GPIO::lcdCLK,
           &GPIO::lcdRST,
           &g_font3x5, clrTransparent, colorText);

    drawBackground(&lcd, 0, 0, lcd.getWidth(), lcd.getHeight());

    for(int i = 0;; i++) {
        xsprintf(buf, "Eff:[%+4d %+4d %+4d]", 
            effector.getX(), 
            effector.getY(),
            effector.getZ());

        // draw the effector
        int16_t x = effector.getX()/2 + lcd.getWidth()/2;
        int16_t y = effector.getZ()/2 + lcd.getHeight()/2;
        int16_t z = 2 + (effector.getY()-180)/16;
        if (z < 0) z = 2;
        if (z > 10) z = 10;

        // clear the area under text
        drawBackground(&lcd, 0,0, lcd.getWidth(), 15);
        // clear the old effector position
        drawEffector(&lcd, px, py, pz, TRUE);

        // draw new effector position
        drawEffector(&lcd, x, y, z);
        px = x, py = y, pz = z;

        // draw text
        lcd.Print(buf, 1, FALSE);

        xsprintf(buf, "Angles:[%+4d %+4d %+4d]", 
                                effector.getAngle0(),
                                effector.getAngle120(),
                                effector.getAngle240());
        lcd.Print(buf, 2, FALSE);

        // check the command queue
        if (xQueueReceive(qhLCD, &qcmd, portMAX_DELAY) == pdTRUE) {
            switch (qcmd) {
            case 0: // set contrast
                    xQueueReceive(qhLCD, &qcmd, portMAX_DELAY);
                    lcd.SetVolume(0, qcmd);
                    break;
            case 1: // invalidate
                    for(;qcmd == 1 && xQueuePeek(qhLCD, &qcmd, 0) == pdTRUE;) {
                        if (qcmd == 1) {
                            xQueueReceive(qhLCD, &qcmd, 0); // purge extra invalidate requests
                        }
                    }
                    break;
            }
        }

        vTaskDelay(80/portTICK_RATE_MS);
    }
}
