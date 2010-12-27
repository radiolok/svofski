#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "gpio.h"
#include "leds.h"
#include "Fonts.h"

static portTASK_FUNCTION_PROTO( ledFlashTask, pvParameters );

static int nled = 0;

void startLEDFlashTasks(unsigned portBASE_TYPE uxPriority) {
    int i;

    for (i = 0; i < 2; i++) {
        xTaskCreate(ledFlashTask, (signed char *) "LEDx", configMINIMAL_STACK_SIZE, NULL, uxPriority, (xTaskHandle *)NULL);
    }
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


