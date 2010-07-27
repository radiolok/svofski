#include <inttypes.h>
#include "lpc210x.h"

#include "common.h"
#include "serialport.h"
#include "serialISR.h"

/*-----------------------------------------------------------*/

/* Constants to setup and access the UART. */
#define serDLAB                         ( ( unsigned char ) 0x80 )
#define serENABLE_INTERRUPTS            ( ( unsigned char ) 0x03 )
#define serNO_PARITY                    ( ( unsigned char ) 0x00 )
#define ser1_STOP_BIT                   ( ( unsigned char ) 0x00 )
#define ser8_BIT_CHARS                  ( ( unsigned char ) 0x03 )
#define serFIFO_ON                      ( ( unsigned char ) 0x01 )
#define serCLEAR_FIFO                   ( ( unsigned char ) 0x06 )
#define serWANTED_CLOCK_SCALING         ( ( unsigned long ) 16 )

#define serINVALID_QUEUE                ( ( xQueueHandle ) 0 )
#define serHANDLE                       ( ( xComPortHandle ) 1 )
#define serNO_BLOCK                     ( ( portTickType ) 0 )

#define UART0_PINSEL                    _BV2(0,2)
#define UART0_PINSEL_M                  _BV4(0,1,2,3)

#define UART1_PINSEL                    (_BV2(16,18)|_BV2(20,22))
#define UART1_PINSEL_M                  (_BV4(16,17,18,19)|_BV4(20,21,22,23))

//SerialPort::SerialPort(uint8_t portId, eBaud baud, eParity par, eDataBits data, eStopBits stop, uint32_t queueLength):
SerialPort::SerialPort(uint8_t portId, uint32_t baudrate, uint32_t queueLength) 
    : id(portId), 
      isr(id, id ? &UART1_IIR : &UART0_IIR,
              id ? &UART1_LSR : &UART0_LSR,
              id ? &UART1_RBR : &UART0_RBR,
              id ? &UART1_THR : &UART0_THR)
    {

    isr.CreateQueues(queueLength);

    if ((isr.getRX() != serINVALID_QUEUE) &&
        (isr.getTX() != serINVALID_QUEUE) &&
        (baudrate != 0UL)) 
    {
        Init(baudrate);
    }
}

void SerialPort::Init(uint32_t baudrate) {
    REG32_T DLL = id ? &UART1_DLL : &UART0_DLL;
    REG32_T DLM = id ? &UART1_DLM : &UART0_DLM;
    REG32_T FCR = id ? &UART1_FCR : &UART0_FCR;
    REG32_T LCR = id ? &UART1_LCR : &UART0_LCR;
    REG32_T IER = id ? &UART1_IER : &UART0_IER;
            THR = id ? &UART1_THR : &UART0_THR;

    portENTER_CRITICAL();

    switch(id) {   
    case 0:
            PCB_PINSEL0 = (PCB_PINSEL0 & ~UART0_PINSEL_M) | UART0_PINSEL;
            break;
    case 1:
            PCB_PINSEL0 = (PCB_PINSEL0 & ~UART1_PINSEL_M) | UART1_PINSEL;
            break;
    }

    /* Setup the baud rate:  Calculate the divisor value. */
    uint32_t ulWantedClock = baudrate * serWANTED_CLOCK_SCALING;
    uint32_t ulDivisor = configCPU_CLOCK_HZ / ulWantedClock;

    /* Set the DLAB bit so we can access the divisor. */
    *LCR |= serDLAB;

    /* Setup the divisor. */
    *DLL = ( unsigned char ) ( ulDivisor & ( unsigned long ) 0xff );
    ulDivisor >>= 8;
    *DLM = ( unsigned char ) ( ulDivisor & ( unsigned long ) 0xff );

    /* Turn on the FIFO's and clear the buffers. */
    *FCR = ( serFIFO_ON | serCLEAR_FIFO );

    /* Setup transmission format. */
    *LCR = serNO_PARITY | ser1_STOP_BIT | ser8_BIT_CHARS;

    isr.Install();

    /* Enable UART0 interrupts. */
    *IER |= serENABLE_INTERRUPTS;

    portEXIT_CRITICAL();
}

int SerialPort::PutChar(char c, portTickType blockTime)
{
    int result = RES_OK;

    portENTER_CRITICAL();
    {
        /* Is there space to write directly to the UART? */
        if (isr.isEmpty() == (long)pdTRUE)
        {
            /* We wrote the character directly to the UART, so was
               successful. */
            isr.setEmpty(pdFALSE);
            *THR = c;
            result = 0;
        }
        else
        {
            /* We cannot write directly to the UART, so queue the character.
            Block for a maximum of xBlockTime if there is no space in the
            queue. */
            result = xQueueSend(isr.getTX(), &c, blockTime ) == pdTRUE ? 0 : -1;

            /* Depending on queue sizing and task prioritisation:  While we
            were blocked waiting to post interrupts were not disabled.  It is
            possible that the serial ISR has emptied the Tx queue, in which
            case we need to start the Tx off again. */
            if( (isr.isEmpty() == (long) pdTRUE) && (result == 0) )
            {
                xQueueReceive(isr.getTX(), &c, serNO_BLOCK);
                isr.setEmpty(pdFALSE);
                *THR = c;
            }
        }
    }
    portEXIT_CRITICAL();

    return result;
}

int SerialPort::GetChar(portTickType blockTime)
{
    char c;

    /* Get the next character from the buffer.  Return false if no characters
       are available, or arrive before xBlockTime expires. */

    if (xQueueReceive(isr.getRX(), &c, blockTime)) {
        return (int)c;
    }

    return -1;
}

int SerialPort::Puts(const char* s, portTickType blockTime)
{
    int result = 0;

    for(;*s && !result; result = PutChar(*s++, blockTime));

    return result;
}
