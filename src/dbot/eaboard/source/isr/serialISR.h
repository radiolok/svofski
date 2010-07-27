#pragma once

#include <inttypes.h>
#include "lpc210x.h"
#include "common.h"

class SerialISR;

class SerialISR {
private:
    static SerialISR* Instances[2];

public:
    SerialISR(uint8_t id, REG32_T regIIR, REG32_T regLSR, REG32_T regRBR, REG32_T regTHR);

    void Install();

    void CreateQueues(uint32_t queueLength);

    inline xQueueHandle getRX(void) { return qhRX; }
    inline xQueueHandle getTX(void) { return qhTX; }
    inline long isEmpty(void) { return lTHREEmpty; }
    inline void setEmpty(long volatile flag) { lTHREEmpty = flag; }

    inline uint32_t getCounter(void) { return counter; }
    
    static void ISR_Wrapper0(void) __attribute__ ((naked));
    static void ISR_Wrapper1(void) __attribute__ ((naked));

    void ISR_Handler(void) __attribute__ ((noinline));
private:
    /* Queues used to hold received characters, and characters waiting to be
       transmitted. */
    xQueueHandle qhRX;
    xQueueHandle qhTX;

    uint8_t portId;
    REG32_T IIR;
    REG32_T LSR;
    REG32_T RBR;
    REG32_T THR;
    volatile long lTHREEmpty;
    volatile uint32_t counter;
};
