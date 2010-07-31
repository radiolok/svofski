#include <inttypes.h>
#include <math.h>
#include "FreeRTOS.h"
#include "servor.h"
#include "effector.h"
#include "gpio.h"
#include "armmodel.h"
#include "xprintf.h"

#define HIP     150.0f
#define ANKLE   350.0f
#define BASE    35.0f
#define EFF     20.0f

Effector::Effector(void) :
    servor(&GPIO::servo[0], &GPIO::servo[1], &GPIO::servo[2]),
    arm0  (0.0f,    HIP, ANKLE, BASE, EFF),
    arm120(120.0f,  HIP, ANKLE, BASE, EFF),
    arm240(240.0f,  HIP, ANKLE, BASE, EFF)
{
    //SetGoal(0, 280, 0); // should put all three servos in near zero position
}

void Effector::SetGoal(int32_t x, int32_t y, int32_t z) 
{
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
}

uint16_t Effector::toServoValue(float rho) {
    return (uint16_t)(1500 - roundf(MathUtil::deg(rho)*4.0f)); 
}
