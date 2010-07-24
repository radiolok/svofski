#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"

#include "gpio.h"
#include "leds.h"

static portTASK_FUNCTION_PROTO( ledFlashTask, pvParameters );

static int nled = 0;

void startLEDFlashTasks(unsigned portBASE_TYPE uxPriority) {
    int i;

    for (i = 0; i < 3; i++) {
        xTaskCreate(ledFlashTask, "LEDx", configMINIMAL_STACK_SIZE, NULL, uxPriority,  (xTaskHandle *)NULL);
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
        gpio_set_led(led_id, 1);
        vTaskDelayUntil(&xLastFlashTime, xFlashRate);
        gpio_set_led(led_id, 0);
        vTaskDelayUntil(&xLastFlashTime, 1/portTICK_RATE_MS);
    }
}

