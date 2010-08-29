#pragma once

#include <inttypes.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "common.h"
#include "serialISR.h"

/// \brief Serial port interface
///
/// This interface is based on example provided with FreeRTOS,
/// but it supports two independent UARTS. The complementary
/// interrupt handler part is located in SerialISR.
class SerialPort {
public:
    /// Constructs a serial port instance.
    /// The constructor also creates SerialISR instances
    /// required for interrupt handling.
    /// \param portId selects UART0 or UART1 
    /// \param baudrate integer value like 115200
    /// \queueLength length of character queue
    SerialPort(uint8_t portId, uint32_t baudrate, uint32_t queueLength);

    /// Put a single char, by default with blocking if queue is full.
    int PutChar(char c, portTickType blockTime = portMAX_DELAY);

    /// Get a single char, by default with blocking if queue is full.
    int GetChar(portTickType blockTime = portMAX_DELAY) const;

    /// Return true if there is something, no wait
    int Peek(portTickType blockTime = portMAX_DELAY) const;

    /// Puts a string in port, possibly blocking if queue is full.
    int Puts(const char* s, portTickType blockTime = portMAX_DELAY);

    /// Get interrupt counter, for debugging.
    inline uint32_t getCounter(void) { return isr.getCounter(); }

private:
    /// Init UART and install ISR handler
    void Init(uint32_t);

    uint8_t id;             /// this instance UART id: 0 or 1
    REG32_T THR;            /// pointer to THR register
    SerialISR isr;          /// ISR instance that handles this port
};
