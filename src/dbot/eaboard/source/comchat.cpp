#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <ctype.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "serialport.h"
#include "comchat.h"
#include "xprintf.h"
#include "display.h"
#include "effector.h"
#include "motion.h"

#define STACK_SIZE      (configMINIMAL_STACK_SIZE*2)
#define BUFFER_LENGTH   64

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

static Waypoint bridge[] = {
    Waypoint(Point(100,240,0),  MotionPath::ACCEL_DECEL),
    Waypoint(Point(100,350,0),  MotionPath::CONST_S),
    Waypoint(Point(100,240,0),  MotionPath::CONST_S),
    Waypoint(Point(-100,240,0), MotionPath::ACCEL_DECEL),
    Waypoint(Point(-100,350,0), MotionPath::CONST_S),
    Waypoint(Point(-100,240,0), MotionPath::CONST_S)
    };

#define MOUSEQ_LEN 16
static Waypoint mousePathWaypoints[MOUSEQ_LEN];

Multipath mousePath(mousePathWaypoints, MOUSEQ_LEN);

int readInt();

int readInt() {
    char c;
    char rbuf[10];
    int i;

    for (i = 0; i < 9; i++) {
        c = serial.GetChar();
        serial.PutChar(c);
        if (c == '-' || isdigit(c)) {
            rbuf[i] = c;
        } else {
            rbuf[i] = '\0';
            break;
        }
    }
    rbuf[9] = '\0';
    //xprintf("read[%s]", rbuf);

    return atoi(rbuf);
}

#define LENGTH(x)   (sizeof(x)/sizeof(x[0]))

static portTASK_FUNCTION( comRxTask, pvParameters ) { (void)pvParameters;
    extern Effector effector;

    int mouseX = 0, mouseY = 0, mouseZ = 0;

    int ic;
    char c = 0;

    uint8_t contrast = 33;

    GPIO0_IOSET = _BV(26); // BT_RST#

    float cal_scale = effector.getScale();
    int16_t cal_base  = effector.getZero();
    bool calibrated = FALSE;

    for(;;) {
        ic = serial.GetChar(10/portTICK_RATE_MS);
        if (ic != -1) {
            c = (char)ic;
            serial.PutChar(c);
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
            case 'b':   
                        while (effector.isMoving()) vTaskDelay(25/portTICK_RATE_MS);

                        if (!effector.isMoving()) {
                            VectorPath zp1(Point(effector.getX(),effector.getY(),effector.getZ()), 
                                           Point(effector.getX(),210,effector.getZ()));
                            effector.Enqueue(&zp1);
                            vTaskDelay(25/portTICK_RATE_MS);
                            while (effector.isMoving()) vTaskDelay(25/portTICK_RATE_MS);
                        }

                        while(serial.GetChar(0) == -1) {
                            for (int phase = 0; phase < LENGTH(bridge); phase++) {
                                VectorPath vp1(Point(effector.getX(), effector.getY(), effector.getZ()), bridge[phase].loc);
                                vp1.SetVelocity(bridge[phase].profile);

                                effector.Enqueue(&vp1);
                                vTaskDelay(25/portTICK_RATE_MS);
                                while (effector.isMoving()) vTaskDelay(10/portTICK_RATE_MS);
                                if (serial.GetChar(0) != -1) goto boo;
                            }

                            for (int i = 0; i < LENGTH(bridge); i++) {
                                bridge[i].loc.rotateY(MathUtil::rad(90));
                            }
                        }
                        boo:
                        break;
            case 'M':
                        {
                            mouseX = readInt();
                            mouseY = readInt();
                            mouseZ = readInt();
                            if (!effector.isMoving()) {
                                mousePath.newPoint(effector.getX(), effector.getY(), effector.getZ());
                            }
                            mousePath.newPoint(mouseX, mouseY, mouseZ);
                            if (!effector.isMoving()) {
                                effector.Enqueue(&mousePath);
                            }
                        }
                       break;

            case '0':   if (!effector.isMoving()) {
                            VectorPath zp1(Point(effector.getX(),effector.getY(),effector.getZ()), Point(0,210,0));
                            effector.Enqueue(&zp1);
                        }
                        break;
            case '1':   if (!effector.isMoving()) {
                            VectorPath qvp1(Point(effector.getX(),effector.getY(),effector.getZ()), Point(100,210,0));
                            CirclePath qcp1(Point(0,210,0), 100, 0, 359);
                            effector.Enqueue(&qvp1);
                            effector.Enqueue(&qcp1);
                        }
                        break;
            case '2':   {
                        }
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
                //btooth.PutChar('\r');
            }
        } else {
            /*
            if (mouseHead != mouseTail) {
                if (!effector.isMoving()) {
                    VectorPath qvp1(Point(effector.getX(), effector.getY(), effector.getZ()), 
                                    mousePath[mouseTail].loc);
                    qvp1.SetVelocity(MotionPath::CONST_S);
                    effector.Enqueue(&qvp1);
                    mouseTail = mouseTail == MOUSEQ_LEN-1 ? 0 : mouseTail + 1;
                }
            }
            */
        }
    }
}

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


