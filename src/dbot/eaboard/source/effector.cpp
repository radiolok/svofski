#include <inttypes.h>
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "servor.h"
#include "effector.h"
#include "gpio.h"
#include "armmodel.h"
#include "xprintf.h"

static portTASK_FUNCTION_PROTO(effControlTask, pvParameters);

#define HIP     155.0f
#define ANKLE   335.0f
#define BASE    85.0f
#define EFF     35.0f

Effector* Effector::Instance;

Effector::Effector(void) :
    servor(&GPIO::servo[0], &GPIO::servo[1], &GPIO::servo[2]),
    arm0  (0.0f,    HIP, ANKLE, BASE, EFF),
    arm120(120.0f,  HIP, ANKLE, BASE, EFF),
    arm240(240.0f,  HIP, ANKLE, BASE, EFF)
{
    Instance = this;
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
    xTaskCreate(effControlTask, (signed char *) "effi", 
                configMINIMAL_STACK_SIZE, NULL, servorPriority-1, (xTaskHandle *) NULL);
}

uint16_t Effector::toServoValue(float rho) const {
    //int16_t ir = roundf(MathUtil::deg(rho)*10);
    //xprintf("[%d]", ir);
    return (uint16_t)(1500 - roundf(MathUtil::deg(rho)*4.0f)); 
}

static portTASK_FUNCTION(effControlTask, pvParameters) { (void)pvParameters;
    int dx, dy, dz;
    int fire = 0;

    Effector *eff;
    extern xQueueHandle qhLCD;
    static const uint8_t cmdLCDInvalidate = 1;

    eff = Effector::Instance;

    vTaskDelay(1000/portTICK_RATE_MS);

    eff->SetGoal(-40, 280, 0);
    xQueueSend(qhLCD, &cmdLCDInvalidate, portMAX_DELAY);

    for(;;) {
        dx = dy = dz = 0;

        if (GPIO::joy[JOY_UP].FPressed())        dz = -4;
        else if (GPIO::joy[JOY_DOWN].FPressed()) dz = 4;

        if (GPIO::joy[JOY_LEFT].FPressed())      dx = -4;
        else if (GPIO::joy[JOY_RIGHT].FPressed())dx = 4;

        if (GPIO::joy[JOY_FIRE].FPressed()) {
            if (fire == 0) {
                fire = 1;

                if (eff->Instance->getY() != 280) {
                    dy = 280 - eff->Instance->getY();
                } else {
                    dy = 180 - eff->Instance->getY();
                }
            }
        } else {
            fire = 0;   
        }

        if (dx != 0 || dz != 0 || dy != 0) {
            eff->SetGoal(eff->Instance->getX() + dx, 
                         eff->Instance->getY() + dy, 
                         eff->Instance->getZ() + dz);

            // request screen redraw
            xQueueSend(qhLCD, &cmdLCDInvalidate, portMAX_DELAY);
        }

        vTaskDelay(25/portTICK_RATE_MS);
    }
}
