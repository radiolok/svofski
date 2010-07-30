#pragma once
#include <inttypes.h>
#include "FreeRTOS.h"
#include "servor.h"

class Effector {
public:
    Effector(void);
    void Init(uint32_t servorPriority);

    void SetGoal(int32_t x, int32_t y, int32_t z);

private:
    Servor servor;
};

