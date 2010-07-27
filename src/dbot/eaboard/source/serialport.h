#pragma once

#include <inttypes.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "common.h"
#include "serialISR.h"

class SerialPort {
public:
    SerialPort(uint8_t portId, uint32_t baudrate, uint32_t queueLength);

    int PutChar(char c, portTickType blockTime = portMAX_DELAY);
    int GetChar(portTickType blockTime = portMAX_DELAY);

    int Puts(const char* s, portTickType blockTime = portMAX_DELAY);

    inline uint32_t getCounter(void) { return isr.getCounter(); }

private:
    void Init(uint32_t);

    uint8_t id;
    REG32_T THR;
    SerialISR isr;
};
