#pragma once

#include <inttypes.h>
#include "lpc210x.h"
#include "common.h"

class SerialISR;

/// \brief UART interrupt handlers and queue dispatchers.
///
/// This class is compiled in ARM mode.
/// The instances are kept in the static array Instances[]
/// Two separate UART interrupt handlers dispatch control
/// each to its corresponding class instance.
///
/// See SerialPort for higher level interface.
class SerialISR {
private:
    static SerialISR* Instances[2];     /// instances of SerialISR for the interrupt handlers to dispatch to

public:
    /// Creates a new UART interrupt handler
    /// \param id UART id: 0 or 1
    /// \param regIIR, regLSR, regRBR, regTHR UART control regs
    SerialISR(uint8_t id, REG32_T regIIR, REG32_T regLSR, REG32_T regRBR, REG32_T regTHR);

    /// Install interrupt handler for this UART
    void Install();

    /// Create TX and RX queues
    void CreateQueues(uint32_t queueLength);

    /// RX queue handle
    inline xQueueHandle getRX(void) const { return qhRX; }    

    /// TX queue handle
    inline xQueueHandle getTX(void) { return qhTX; }    

    /// Checks if THRE is empty
    inline long isEmpty(void) { return lTHREEmpty; }    

    /// Sets the empty flag
    inline void setEmpty(long volatile flag) { lTHREEmpty = flag; } 

    inline uint32_t getCounter(void) { return counter; }

    /// ISR wrapper for UART0
    static void ISR_Wrapper0(void) __attribute__ ((naked)); 

    /// ISR wrapper for UART1
    static void ISR_Wrapper1(void) __attribute__ ((naked)); 

    /// Actual ISR handler
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
