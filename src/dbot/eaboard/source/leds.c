#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"

#include "gpio.h"
#include "leds.h"
#include "LCD_Driver.h"
#include "serial.h"

static portTASK_FUNCTION_PROTO( ledFlashTask, pvParameters );

static portTASK_FUNCTION_PROTO( lcdControlTask, pvParameters);

static int nled = 0;

void startLEDFlashTasks(unsigned portBASE_TYPE uxPriority) {
    int i;

    for (i = 0; i < 2; i++) {
        xTaskCreate(ledFlashTask, "LEDx", configMINIMAL_STACK_SIZE, NULL, uxPriority,  (xTaskHandle *)NULL);
    }
    gpio_set_led(0,1);
    gpio_set_led(1,1);
    gpio_set_led(2,1);
    gpio_set_led(3,1);

    xTaskCreate(lcdControlTask, "LCD", configMINIMAL_STACK_SIZE, NULL, uxPriority, (xTaskHandle *)NULL);
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
        gpio_set_led(led_id, 0);
        vTaskDelayUntil(&xLastFlashTime, 100/portTICK_RATE_MS);
        gpio_set_led(led_id, 1);
        vTaskDelayUntil(&xLastFlashTime, xFlashRate);
        //vTaskDelete(NULL);
    }
}

static const int color[4] = {RED, GREEN, BLUE, WHITE};

static portTASK_FUNCTION(lcdControlTask, pvParameters) {
    int i;

    vTaskDelay(500/portTICK_RATE_MS);

    xSerialPutChar(NULL, '1', 0);
    LCDInit();
    xSerialPutChar(NULL, '2', 0);
    LCDClear(BLUE);

    LCDSetPixel(YELLOW, 0, 0);
    LCDSetPixel(YELLOW, 1, 1);
    LCDSetPixel(YELLOW, 2, 2);
    LCDSetPixel(YELLOW, 3, 3);
    LCDSetPixel(RED, 128, 128);
    LCDSetPixel(RED, 129, 129);
    LCDSetPixel(RED, 131, 131);
    LCDSetPixel(RED, 132, 132);

    for(;;i++) {
        LCDClear(BLUE);

        LCDFillRect(10,10,20,10, RED);

        LCDFillRect(10+i,30+i,10,20, MAGENTA);

        vTaskDelay(50/portTICK_RATE_MS);
    }
}
