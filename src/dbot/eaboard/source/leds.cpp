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
    gpio_set_led(0,1);
    gpio_set_led(1,1);
    gpio_set_led(2,1);
    gpio_set_led(3,1);

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
        gpio_set_led(led_id, 0);
        vTaskDelayUntil(&xLastFlashTime, 100/portTICK_RATE_MS);
        gpio_set_led(led_id, 1);
        vTaskDelayUntil(&xLastFlashTime, xFlashRate);
        //vTaskDelete(NULL);
    }
}

#define LCD_RES (1<<25)// LCD_RST#
#define LCD_CS  (1<<24)// LCD_CS#
#define LCD_DIO (1<<6) // MOSI
#define LCD_SCK (1<<4) // CLK


static OUT lcdCS   = OUT(LCD_CS, 1);
static OUT lcdMOSI = OUT(LCD_DIO, 0);
static OUT lcdSCK  = OUT(LCD_SCK, 0);
static OUT lcdRES  = OUT(LCD_RES, 1);

char buf[64];

static portTASK_FUNCTION(lcdControlTask, pvParameters) {
    vTaskDelay(500/portTICK_RATE_MS);

    LCD lcd(&lcdCS,
            &lcdMOSI,
            &lcdSCK,
            &lcdRES,
            &g_font3x5, clrYellow, clrDkBlue);
            
   for(int i = 0;;i++) {
        lcd.FillRect(90,10,20,20, i & 0377);
        lcd.FillCircle(96,64, 1+ (16 - i % 16), 0377 - i & 0377);

        xsprintf(buf, "Gruuu %010d", i);
        lcd.Print(buf, 1 + i % 20);

        vTaskDelay(100/portTICK_RATE_MS);
    }
}
