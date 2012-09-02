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
    Timer1(OUT* pin1, OUT* pin2, OUT* pin3);
    void Install(void);

    void Load(uint32_t time1, uint32_t time2, uint32_t time3);

    void Enable(bool enable);

    inline uint32_t getCounter() const { return counter; }

private:
    void Run();

    static void ISR_Wrapper(void) __attribute__ ((naked));

    void ISR_Handler(void) __attribute__ ((noinline));
private:
    OUT* pin[3];
    volatile uint32_t counter;
    uint32_t time[3];
    uint8_t pinidx;
};
