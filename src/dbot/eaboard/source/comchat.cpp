#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "serialport.h"
#include "comchat.h"
#include "xprintf.h"
#include "display.h"
#include "effector.h"

#define STACK_SIZE      (configMINIMAL_STACK_SIZE*2)
#define BUFFER_LENGTH   8

static portTASK_FUNCTION_PROTO( comTxTask, pvParameters );
static portTASK_FUNCTION_PROTO( comRxTask, pvParameters );
static portTASK_FUNCTION_PROTO( btRxTask,  pvParameters );

#define LINE_LEN 64
static char lineBuffer[LINE_LEN];

xSemaphoreHandle semaphore;

SerialPort serial(0, 230400, BUFFER_LENGTH);
SerialPort btooth(1, 115200, BUFFER_LENGTH);

void createSerialChatTasks(unsigned portBASE_TYPE priority) {
    vSemaphoreCreateBinary(semaphore);

    xTaskCreate(comTxTask, (signed char *) "COMtx", STACK_SIZE, NULL, priority - 1, (xTaskHandle *) NULL);
    xTaskCreate(comRxTask, (signed char *) "COMrx", STACK_SIZE, NULL, priority, (xTaskHandle *) NULL);
    xTaskCreate(btRxTask,  (signed char *) "BTrx",  STACK_SIZE, NULL, priority, (xTaskHandle *) NULL);
}

static portTASK_FUNCTION( comTxTask, pvParameters ) { (void)pvParameters;
    vTaskDelay(100/portTICK_RATE_MS); // yield for the rx to take the semaphore first
    for(;;) {
        if (xSemaphoreTake(semaphore, portMAX_DELAY) == pdTRUE) {
            // dump the input line
            if (lineBuffer[0] != 0) {
                serial.Puts("You say: ");
                serial.Puts(lineBuffer);
                serial.Puts("\n");
            }
        } else {
            serial.PutChar('.');
        }
    }
}

static portTASK_FUNCTION( comRxTask, pvParameters ) { (void)pvParameters;
    extern Effector effector;

    int ic;
    char c = 0;

    uint8_t contrast = 33;

    GPIO0_IOSET = _BV(26); // BT_RST#

    float cal_scale = effector.getScale();
    int16_t cal_base  = effector.getZero();
    bool calibrated = FALSE;

    for(;;) {
        ic = serial.GetChar();
        if (ic != -1) {
            c = (char)ic;
            switch(c) {
            case '[':   contrast--;
                        display.Enqueue(CMD_LCD_CONTRAST, contrast);
                        xprintf("Contrast=%d\n", contrast);
                        break;
            case ']':   contrast++;
                        display.Enqueue(CMD_LCD_CONTRAST, contrast);
                        xprintf("Contrast=%d\n", contrast);
                        break;
            case 's':   cal_scale += 0.05; calibrated = TRUE;
                        break;
            case 'S':   cal_scale -= 0.05; calibrated = TRUE;
                        break;
            case 'z':   cal_base++; calibrated = TRUE;
                        break;
            case 'Z':   cal_base--; calibrated = TRUE;
                        break;
            case 'j':   effector.SetGoal(effector.getX(), effector.getY()+5, effector.getZ());
                        display.Enqueue(CMD_LCD_INVALIDATE);
                        break;
            case 'J':   effector.SetGoal(effector.getX(), effector.getY()+10, effector.getZ());
                        display.Enqueue(CMD_LCD_INVALIDATE);
                        break;
            case 'k':   effector.SetGoal(effector.getX(), effector.getY()-5, effector.getZ());
                        display.Enqueue(CMD_LCD_INVALIDATE);
                        break;
            case 'K':   effector.SetGoal(effector.getX(), effector.getY()-10, effector.getZ());
                        display.Enqueue(CMD_LCD_INVALIDATE);
                        break;
            case '0':   effector.zero();
                        break;
            case '1':   effector.lol(Effector::LOOP1);
                        break;
            case '2':   effector.lol(Effector::LOOP2);
                        break;
            default:    break;
            }

            if (calibrated) {
                //effector...;
                effector.calibrate(cal_base, cal_scale);
                xprintf("Z=%d S=%d.%02d\n", cal_base, (int)floorf(cal_scale), (int)roundf(cal_scale*100)%100);
            }

            if (c == '\r') xprintf("\\r");
            if (c == '\n') {
                xprintf("\\n");
                btooth.PutChar('\r');
            }
            btooth.PutChar(c);
        }
    }
}
/*
            serial.PutChar(c);
            

            btooth.PutChar(c);
            ib = btooth.GetChar(1000);
            serial.PutChar(ib == -1 ? '#' : (char)ib);

            if (c == '%') GPIO0_PIN ^= _BV(26);
    
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
*/

static portTASK_FUNCTION( btRxTask, pvParameters ) { (void)pvParameters;
    int idx = 0;
    int ic;
    char c = 0;

    for(;;) {
        ic = btooth.GetChar();
        if (ic != -1) {
            c = (char)ic;
            serial.PutChar(c);
        }
    }
}

extern "C" void xputchar(char c);

void xputchar(char c) {
   serial.PutChar(c);
}


