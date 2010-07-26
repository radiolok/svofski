#include <inttypes.h>
#include "lpc210x.h"
#include "serialport.h"

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

/* Constants to setup and access the VIC. */
#define serUART0_VIC_CHANNEL            ( ( unsigned long ) 0x0006 )
#define serUART0_VIC_CHANNEL_BIT        ( ( unsigned long ) 0x0040 )
#define serUART0_VIC_ENABLE             ( ( unsigned long ) 0x0020 )
#define serCLEAR_VIC_INTERRUPT          ( ( unsigned long ) 0 )

#define serUART1_VIC_CHANNEL            ( ( unsigned long ) 0x0007 )
#define serUART1_VIC_CHANNEL_BIT        ( ( unsigned long ) 0x0080 )
#define serUART1_VIC_ENABLE             ( ( unsigned long ) 0x0020 )

#define serINVALID_QUEUE                ( ( xQueueHandle ) 0 )
#define serHANDLE                       ( ( xComPortHandle ) 1 )
#define serNO_BLOCK                     ( ( portTickType ) 0 )


//SerialPort::SerialPort(uint8_t portId, eBaud baud, eParity par, eDataBits data, eStopBits stop, uint32_t queueLength):
SerialPort::SerialPort(uint8_t portId, uint32_t baudrate, uint32_t queueLength) : id(portId) {
    Init(baudrate, queueLength);
}

void SerialPort::Init(uint32_t baudrate, uint32_t queueLength) {
    /* The queues are used in the serial ISR routine, so are created from
    serialISR.c (which is always compiled to ARM mode. */

    vSerialISRCreateQueues(queueLength, &qhRX, &qhTX, &plFlag);

    REG32_T DLL = id ? &UART1_DLL : &UART0_DLL;
    REG32_T DLM = id ? &UART1_DLM : &UART0_DLM;
    REG32_T FCR = id ? &UART1_FCR : &UART0_FCR;
    REG32_T LCR = id ? &UART1_LCR : &UART0_LCR;
    REG32_T IER = id ? &UART1_IER : &UART0_IER;
            THR = id ? &UART1_THR : &UART0_THR;


    if(
        ( qhRX != serINVALID_QUEUE ) &&
        ( qhTX != serINVALID_QUEUE ) &&
        ( baudrate != ( unsigned long ) 0 )
      )
    {
        portENTER_CRITICAL();
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

        // TODO for the second uart

        /* Setup the VIC for the UART. */
        switch (id) {
        case 0:
            VICIntSelect &= ~( serUART0_VIC_CHANNEL_BIT );
            VICIntEnable |= serUART0_VIC_CHANNEL_BIT;
            VICVectAddr1 = (long) vUART_ISR_Wrapper;
            VICVectCntl1 = serUART0_VIC_CHANNEL | serUART0_VIC_ENABLE;
            break;
        case 1:
            VICIntSelect &= ~( serUART1_VIC_CHANNEL_BIT );
            VICIntEnable |= serUART1_VIC_CHANNEL_BIT;
            VICVectAddr2 = (long) vUART_ISR_Wrapper;
            VICVectCntl2 = serUART0_VIC_CHANNEL | serUART0_VIC_ENABLE;
            break;
        }

        /* Enable UART0 interrupts. */
        *IER |= serENABLE_INTERRUPTS;

        portEXIT_CRITICAL();
    }
}

int SerialPort::PutChar(char c, portTickType blockTime) const
{
    int result = RES_OK;

    portENTER_CRITICAL();
    {
        /* Is there space to write directly to the UART? */
        if( *plFlag == ( long ) pdTRUE )
        {
            /* We wrote the character directly to the UART, so was
            successful. */
            *plFlag = pdFALSE;
            *THR = c;
            result = 0;
        }
        else
        {
            /* We cannot write directly to the UART, so queue the character.
            Block for a maximum of xBlockTime if there is no space in the
            queue. */
            result = xQueueSend(qhTX, &c, blockTime ) == pdTRUE ? 0 : -1;

            /* Depending on queue sizing and task prioritisation:  While we
            were blocked waiting to post interrupts were not disabled.  It is
            possible that the serial ISR has emptied the Tx queue, in which
            case we need to start the Tx off again. */
            if( (*plFlag == (long) pdTRUE) && (result == 0) )
            {
                xQueueReceive(qhTX, &c, serNO_BLOCK);
                *plFlag = pdFALSE;
                *THR = c;
            }
        }
    }
    portEXIT_CRITICAL();

    return result;
}

int SerialPort::GetChar(portTickType blockTime) const
{
    char c;

    /* Get the next character from the buffer.  Return false if no characters
       are available, or arrive before xBlockTime expires. */

    if (xQueueReceive(qhRX, &c, blockTime)) {
        return (int)c;
    }

    return -1;
}

int SerialPort::Puts(const char* s, portTickType blockTime) const 
{
    int result = 0;

    for(;*s && !result; result = PutChar(*s++, blockTime));

    return result;
}
