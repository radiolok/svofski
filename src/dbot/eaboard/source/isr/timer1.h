#pragma once

#include <inttypes.h>
#include "lpc210x.h"
#include "common.h"
#include "InOut.h"

#include "FreeRTOS.h"

class Timer1;

class Timer1 {
private:
    static Timer1* Instance;

public:
    Timer1(void);
    void Install(void);

    void RunOnce(uint32_t us, OUT* pin);

private:
    static void ISR_Wrapper(void) __attribute__ ((naked));

    void ISR_Handler(void) __attribute__ ((noinline));
private:
    OUT* pin;
};
