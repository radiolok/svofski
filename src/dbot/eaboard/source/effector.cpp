#include <inttypes.h>
#include "FreeRTOS.h"
#include "servor.h"
#include "effector.h"
#include "gpio.h"
#include "armmodel.h"

Effector::Effector(void) :
    servor(&GPIO::servo[0], &GPIO::servo[1], &GPIO::servo[2])
{
}

void Effector::Init(uint32_t servorPriority) 
{
    servor.CreateTask(servorPriority);

    float hip = 150.0f;
    float ankle = 350.0f;
    float baseSize = 35.0f;
    float effSize = 20.0f;
    ArmModel arm1(0.0f, hip, ankle, baseSize, effSize);
}


