#include <stdlib.h>
#include <inttypes.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "serial.h"
#include "comchat.h"

#define STACK_SIZE      configMINIMAL_STACK_SIZE
#define BUFFER_LENGTH   8

static portTASK_FUNCTION_PROTO( comTxTask, pvParameters );
static portTASK_FUNCTION_PROTO( comRxTask, pvParameters );

static xComPortHandle port = NULL;

#define LINE_LEN 64
static char lineBuffer[LINE_LEN];

xSemaphoreHandle semaphore;

void createSerialChatTasks(unsigned portBASE_TYPE priority, uint32_t baudRate) {
    xSerialPortInitMinimal(baudRate, BUFFER_LENGTH);

    vSemaphoreCreateBinary(semaphore);

    xTaskCreate(comTxTask, (signed char *) "COMtx", STACK_SIZE, NULL, priority - 1, (xTaskHandle *) NULL);
    xTaskCreate(comRxTask, (signed char *) "COMrx", STACK_SIZE, NULL, priority, (xTaskHandle *) NULL);
}

void serial_puts(char *s) {
    for (;*s;) {
        xSerialPutChar(port, *s++, portMAX_DELAY);
    }
}

static portTASK_FUNCTION( comTxTask, pvParameters ) {
    vTaskDelay(100/portTICK_RATE_MS); // yield for the rx to take the semaphore first
    for(;;) {
        if (xSemaphoreTake(semaphore, portMAX_DELAY) == pdTRUE) {
            // dump the input line
            if (lineBuffer[0] != 0) {
                serial_puts("You say: ");
                serial_puts(lineBuffer);
                serial_puts("\n");
                //vSerialPutString(port, (const signed char *) lineBuffer, 64);
                //xSerialPutChar(port, '\n', 0);
            }
        } else {
            xSerialPutChar(port, '.', 0);
        }
    }
}

static portTASK_FUNCTION( comRxTask, pvParameters ) {
    int idx = 0;
    int c = 0;

    for(;;) {
        if (xSerialGetChar(port, &c, portMAX_DELAY)) {
            xSerialPutChar(port, c, 0);
            if (c == '\r') continue;
            if (c == '\n' || (idx + 1) == LINE_LEN) {
                lineBuffer[idx] = '\0';
                idx = 0;
                xSemaphoreGive(semaphore);
            } else {
                lineBuffer[idx] = c;
                idx++;
            }
        }
    }
}

