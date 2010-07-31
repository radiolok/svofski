#pragma once
#include <inttypes.h>
#include "FreeRTOS.h"
#include "servor.h"
#include "armmodel.h"

class Effector {
public:
    Effector(void);
    void Init(uint32_t servorPriority);

    void SetGoal(int32_t x, int32_t y, int32_t z);

    uint16_t toServoValue(float rho);

private:
    Servor servor;

    ArmModel arm0, arm120, arm240;
};

