#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"

#include "gpio.h"
#include "leds.h"
#include "LCD-color.h"
#include "Fonts.h"
#include "xprintf.h"

static portTASK_FUNCTION_PROTO( ledFlashTask, pvParameters );

static portTASK_FUNCTION_PROTO( lcdControlTask, pvParameters);

static int nled = 0;

void startLEDFlashTasks(unsigned portBASE_TYPE uxPriority) {
    int i;

    for (i = 0; i < 2; i++) {
        xTaskCreate(ledFlashTask, (signed char *) "LEDx", configMINIMAL_STACK_SIZE, NULL, uxPriority,  (xTaskHandle *)NULL);
    }

    xTaskCreate(lcdControlTask, (signed char *)"LCD", configMINIMAL_STACK_SIZE, NULL, uxPriority, (xTaskHandle *)NULL);
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


static portTASK_FUNCTION(lcdControlTask, pvParameters) { (void)pvParameters;
    char buf[64];

    vTaskDelay(500/portTICK_RATE_MS);

    LCD lcd(&GPIO::lcdCS,
            &GPIO::lcdMOSI,
            &GPIO::lcdCLK,
            &GPIO::lcdRST,
            &g_font3x5, clrYellow, clrDkBlue);
            
   for(int i = 0;; i++) {
        lcd.FillRect(90,10,20,20, i & 0377);
        lcd.FillCircle(96,64, 1+ (16 - i % 16), 0377 - (i & 0377));

        xsprintf(buf, "Gruuu %010d", i);
        lcd.Print(buf, 1 + i % 20, FALSE);

        vTaskDelay(100/portTICK_RATE_MS);
    }
}
