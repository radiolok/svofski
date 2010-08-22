#include <inttypes.h>
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "gpio.h"
#include "effector.h"
#include "armmodel.h"
#include "xprintf.h"
#include "display.h"
#include "timer1.h"

#include "motion.h"

#define HIP     150.0f
#define ANKLE   335.0f
#define BASE    78.0f
#define EFF     35.0f

#define EFFECTOR_TASK_STACK_SIZE    (configMINIMAL_STACK_SIZE * 4)

Effector effector;

Effector::Effector(void) :
    timer(&GPIO::servo[0], &GPIO::servo[1], &GPIO::servo[2]),
    arm0  (0.0f,    HIP, ANKLE, BASE, EFF),
    arm120(120.0f,  HIP, ANKLE, BASE, EFF),
    arm240(240.0f,  HIP, ANKLE, BASE, EFF),
    cal_zero(1520*4),
    cal_scale(10.14f*4),
    modeReq(HOLD)
{
    timer.Install();
}

void Effector::SetGoal(int32_t x, int32_t y, int32_t z) 
{
    goal_x = x, goal_y = y, goal_z = z;

    float rho0   = arm0.MoveTo(x, y, z);
    float rho120 = arm120.MoveTo(x, y, z);
    float rho240 = arm240.MoveTo(x, y, z);

    timer.Load(toServoValue(rho0),
               toServoValue(rho120),
               toServoValue(rho240));
}

void Effector::Init(uint32_t servorPriority) 
{
    xTaskCreate(controlTask, (signed char *) "EFF", 
                EFFECTOR_TASK_STACK_SIZE, 
                NULL, servorPriority-1, (xTaskHandle *) NULL);
}

uint32_t Effector::toServoValue(float rho) const {
    return (uint32_t)(cal_zero + roundf(MathUtil::deg(rho)*cal_scale)); 
}

void Effector::calibrate(int32_t base, float scale) {
    cal_zero = base;
    cal_scale = scale;
    needUpdate = TRUE;
}

void Effector::zero() {
    SetGoal(0, 350, 0);
    needUpdate = TRUE;
}

void Effector::lol(LolMode lm) {
    modeReq = lm;
}

void Effector::updateLoop() {
    int dx, dy, dz;
    int fire = 0;
    int animate = 0,
        animateGoal = 0,
        animode = 0;

    int loop2angle;
   
    int lx = 0,
        lz = 0;

    int movingtime = 0,
        speed = 3;

    Point zeropoint(0,210,0);
    CirclePath circlePath(zeropoint, 100.0f);

    portTickType loopTime;

    SetGoal(0, 350, 0);

    timer.Enable(TRUE);

    animode = RAISE;
    animate = -1;
    animateGoal = 210;

    display.Enqueue(CMD_LCD_INVALIDATE);

    loopTime = xTaskGetTickCount();

    for(;;) {
        dx = dy = dz = 0;
        taskENTER_CRITICAL();
        animode ^= modeReq;
        modeReq = HOLD;
        taskEXIT_CRITICAL();

        if (GPIO::joy[JOY_UP].FPressed())        dz = -1;
        else if (GPIO::joy[JOY_DOWN].FPressed()) dz = 1;

        if (GPIO::joy[JOY_LEFT].FPressed())      dx = -1;
        else if (GPIO::joy[JOY_RIGHT].FPressed())dx = 1;

        if (GPIO::joy[JOY_FIRE].FPressed()) {
            if (fire == 0) {
                fire = 1;

                if (getY() != 350) {
                    animode |= RAISE;
                    animateGoal = 350;
                    animate = 1;
                } else {
                    animode |= RAISE;
                    animateGoal = 210;
                    animate = -1;
                }
            }
        } else {
            fire = 0;   
        }

        if (animode & RAISE) {
            if (getY() == animateGoal) {
                animate = 0;
                animode ^= RAISE;
            } else {
                dy = animate;
            }
        }

        if (animode & MOVETO) {
            cx += fx; cy += fy; cz += fz;
            xprintf("y=%d |%d|", (int)cy, (int)MathUtil::dist(cx,cy,cz, gx, gy,gz));
            if (MathUtil::dist(cx,cy,cz, gx,gy,gz) < 1.1f) {
                animode ^= MOVETO;
                xprintf("GOAL\n");
            }
            dx = roundf(cx) - getX();
            dy = roundf(cy) - getY();
            dz = roundf(cz) - getZ();
        }

        if (animode & LOOP1) {
            if (loopradius < 100) loopradius++;
        } else {
            if (loopradius > 0) loopradius--;
            if ((animode & LOOP2) == 0) {
                if (loopradius > 50) 
                    speed = 3;
                else 
                    speed = 4;
            }
        }

        if (loopradius > 0) {
            //lx = roundf(cos(((float)loopangle)/180.0*M_PI) * loopradius);
            //lz = roundf(sin(((float)loopangle)/180.0*M_PI) * loopradius);
            //loopangle = (loopangle + 1) % 360;
            Point lp(0,0,0);
            int dummy;
            circlePath.next(&lp, &dummy);
            lx = loopradius/100.0*lp.x;
            lz = loopradius/100.0*lp.z;
            needUpdate = TRUE;
        } else {
            lx = lz = 0;
        }

        if (animode & LOOP2) {
            if (loop2radius < 25) loop2radius++;
        } else {
            if (loop2radius > 0) loop2radius--;
            if ((animode & LOOP1) == 0) {
                speed = 4;
            }
        }

        if (loop2radius > 0) {
            lx += roundf(cos(((float)loop2angle)/180.0*M_PI) * loop2radius);
            lz += roundf(sin(((float)loop2angle)/180.0*M_PI) * loop2radius);
            loop2angle = (loop2angle + 3) % 360;
            needUpdate = TRUE;
        }

        if (loopradius || loop2radius) {
            dx = lx - getX();
            dz = lz - getZ();
        }

        if (dx != 0 || dz != 0 || dy != 0 || needUpdate) {
            SetGoal(getX() + dx, 
                    getY() + dy, 
                    getZ() + dz);

            //xprintf("[%d %d %d]\n", getX(), getY(), getZ());

            // request screen redraw
            display.Enqueue(CMD_LCD_INVALIDATE);
            needUpdate = FALSE;

            if (animode) { 
                movingtime++;
                if (movingtime < 20) speed = 4;
                else if (movingtime < 40) speed = 3;
                else if (movingtime < 60) speed = 2;
            } else {
                speed = 3;
            }

            while(xTaskGetTickCount() < loopTime + speed) taskYIELD();
            loopTime = xTaskGetTickCount();
        } else {
            vTaskDelayUntil(&loopTime, 10/portTICK_RATE_MS);
            movingtime = 0;
        }
    }
}

void Effector::controlTask(void *params) 
{   
    (void)params;

    //vTaskDelay(50/portTICK_RATE_MS);
    effector.updateLoop();
}
