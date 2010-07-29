#pragma once
#include <inttypes.h>
#include "FreeRTOS.h"
#include "servor.h"

class Effector {
public:
    Effector(void);
    void Init(uint32_t servorPriority);

private:
    Servor servor;
};

