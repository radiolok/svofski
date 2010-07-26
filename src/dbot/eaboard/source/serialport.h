#pragma once

#include <inttypes.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "common.h"

/*
 * The queues are created in serialISR.c as they are used from the ISR.
 * Obtain references to the queues and THRE Empty flag.
 */
extern "C" void vSerialISRCreateQueues( unsigned portBASE_TYPE uxQueueLength, xQueueHandle *pxRxedChars, xQueueHandle *pxCharsForTx, long volatile **pplTHREEmptyFlag );

extern "C" void ( vUART_ISR_Wrapper )( void );

class SerialPort {
public:
    SerialPort(uint8_t portId, uint32_t baudrate, uint32_t queueLength);

    int PutChar(char c, portTickType blockTime = portMAX_DELAY) const;
    int GetChar(portTickType blockTime = portMAX_DELAY) const;

    int Puts(const char* s, portTickType blockTime = portMAX_DELAY) const;

private:
    void Init(uint32_t, uint32_t);

    uint8_t id;

    xQueueHandle qhRX;
    xQueueHandle qhTX;
    volatile long* plFlag;       // a magic flag

    REG32_T THR;
};
