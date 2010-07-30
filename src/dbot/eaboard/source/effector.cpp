#include <inttypes.h>
#include "FreeRTOS.h"
#include "servor.h"
#include "effector.h"
#include "gpio.h"

Effector::Effector(void) :
    servor(&GPIO::servo[0], &GPIO::servo[1], &GPIO::servo[2])
{
}

void Effector::Init(uint32_t servorPriority) 
{
    servor.CreateTask(servorPriority);
}


