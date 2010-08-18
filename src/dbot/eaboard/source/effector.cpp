#include <inttypes.h>
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "gpio.h"
#include "servor.h"
#include "effector.h"
#include "armmodel.h"
#include "xprintf.h"
#include "display.h"

#define HIP     150.0f
#define ANKLE   335.0f
#define BASE    78.0f
#define EFF     35.0f

#define EFFECTOR_TASK_STACK_SIZE    (configMINIMAL_STACK_SIZE * 2)

Effector effector;

Effector::Effector(void) :
    servor(&GPIO::servo[0], &GPIO::servo[1], &GPIO::servo[2]),
    arm0  (0.0f,    HIP, ANKLE, BASE, EFF),
    arm120(120.0f,  HIP, ANKLE, BASE, EFF),
    arm240(240.0f,  HIP, ANKLE, BASE, EFF),
    cal_zero(1520),
    cal_scale(8.888f)
{
}

void Effector::SetGoal(int32_t x, int32_t y, int32_t z) 
{
    goal_x = x, goal_y = y, goal_z = z;

    float rho0   = arm0.MoveTo(x, y, z);
    float rho120 = arm120.MoveTo(x, y, z);
    float rho240 = arm240.MoveTo(x, y, z);

    servor.SetPosition(toServoValue(rho0),
                       toServoValue(rho120),
                       toServoValue(rho240));
}

void Effector::Init(uint32_t servorPriority) 
{
    servor.CreateTask(servorPriority);
    xTaskCreate(controlTask, (signed char *) "EFF", 
                EFFECTOR_TASK_STACK_SIZE, 
                NULL, servorPriority-1, (xTaskHandle *) NULL);
}

uint16_t Effector::toServoValue(float rho) const {
    //int16_t ir = roundf(MathUtil::deg(rho)*10);
    //xprintf("[%d]", ir);
    return (uint16_t)(cal_zero - roundf(MathUtil::deg(rho)*cal_scale)); 
}

void Effector::calibrate(int16_t base, float scale) {
    cal_zero = base;
    cal_scale = scale;
    needUpdate = TRUE;
}

void Effector::zero() {
    SetGoal(0, 210, 0);
    needUpdate = TRUE;
}

void Effector::updateLoop() {
    int dx, dy, dz;
    int fire = 0;
    int animate = 0,
        animateGoal = 0;

    SetGoal(0, 210, 0);
    display.Enqueue(CMD_LCD_INVALIDATE);

    for(;;) {
        dx = dy = dz = 0;

        if (GPIO::joy[JOY_UP].FPressed())        dz = -1;
        else if (GPIO::joy[JOY_DOWN].FPressed()) dz = 1;

        if (GPIO::joy[JOY_LEFT].FPressed())      dx = -1;
        else if (GPIO::joy[JOY_RIGHT].FPressed())dx = 1;

        if (GPIO::joy[JOY_FIRE].FPressed()) {
            if (fire == 0) {
                fire = 1;

                if (getY() != 280) {
                    animate = 2;
                    animateGoal = 280;
                    //dy = 280 - getY();
                } else {
                    animate = -2;
                    animateGoal = 210;
                    //dy = 210 - getY();
                }
            }
        } else {
            fire = 0;   
        }

        if (animate) {
            if (getY() == animateGoal) {
                animate = 0;
            } else {
                dy = animate;
            }
        }

        if (dx != 0 || dz != 0 || dy != 0 || needUpdate) {
            SetGoal(getX() + dx, 
                    getY() + dy, 
                    getZ() + dz);

            // request screen redraw
            display.Enqueue(CMD_LCD_INVALIDATE);
            needUpdate = FALSE;
        }

        vTaskDelay(25/portTICK_RATE_MS);
    }
}

void Effector::controlTask(void *params) 
{   
    (void)params;

    //vTaskDelay(50/portTICK_RATE_MS);
    effector.updateLoop();
}
