#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "serialISR.h"

/* Constant to access the VIC. */
#define serCLEAR_VIC_INTERRUPT			( ( unsigned long ) 0 )

/* Constants to determine the ISR source. */
#define serSOURCE_THRE					( ( unsigned char ) 0x02 )
#define serSOURCE_RX_TIMEOUT			( ( unsigned char ) 0x0c )
#define serSOURCE_ERROR					( ( unsigned char ) 0x06 )
#define serSOURCE_RX					( ( unsigned char ) 0x04 )
#define serINTERRUPT_SOURCE_MASK		( ( unsigned char ) 0x0f )

/* Constants to setup and access the VIC. */
#define serUART0_VIC_CHANNEL            ( ( unsigned long ) 0x0006 )
#define serUART0_VIC_CHANNEL_BIT        ( ( unsigned long ) 0x0040 )
#define serCLEAR_VIC_INTERRUPT          ( ( unsigned long ) 0 )

#define serUART_VIC_ENABLE             ( ( unsigned long ) 0x0020 )

#define serUART1_VIC_CHANNEL            ( ( unsigned long ) 0x0007 )
#define serUART1_VIC_CHANNEL_BIT        ( ( unsigned long ) 0x0080 )

SerialISR* SerialISR::Instances[2];

SerialISR::SerialISR(uint8_t id, REG32_T regIIR, REG32_T regLSR, REG32_T regRBR, REG32_T regTHR):
   portId(id), IIR(regIIR), LSR(regLSR), RBR(regRBR), THR(regTHR), counter(0)
{
   lTHREEmpty = (long)pdTRUE;
   SerialISR::Instances[id] = this;
}

void SerialISR::CreateQueues(uint32_t queueLength)
{
	/* Create the queues used to hold Rx and Tx characters. */
	qhRX = xQueueCreate(queueLength, (unsigned portBASE_TYPE) sizeof(char));
	qhTX = xQueueCreate(queueLength + 1, (unsigned portBASE_TYPE) sizeof(char));

	/* Initialise the THRE empty flag - and pass back a reference. */
	lTHREEmpty = (long) pdTRUE;
}

void SerialISR::Install() 
{
    /* Setup the VIC for the UART. */
    switch (portId) {
    case 0:
        VICIntSelect &= ~( serUART0_VIC_CHANNEL_BIT );
        VICIntEnable |= serUART0_VIC_CHANNEL_BIT;
        VICVectAddr1 = (long) SerialISR::ISR_Wrapper0;
        VICVectCntl1 = serUART0_VIC_CHANNEL | serUART_VIC_ENABLE;
        break;
    case 1:
        VICIntSelect &= ~( serUART1_VIC_CHANNEL_BIT );
        VICIntEnable |= serUART1_VIC_CHANNEL_BIT;
        VICVectAddr2 = (long) SerialISR::ISR_Wrapper1;
        VICVectCntl2 = serUART1_VIC_CHANNEL | serUART_VIC_ENABLE;
        break;
    }
}

void SerialISR::ISR_Wrapper0(void)
{
	/* Save the context of the interrupted task. */
	portSAVE_CONTEXT();

	/* Call the handler.  This must be a separate function from the wrapper
	to ensure the correct stack frame is set up. */
	//__asm volatile ("bl vUART_ISR_Handler");
    SerialISR::Instances[0]->ISR_Handler();

	/* Restore the context of whichever task is going to run next. */
	portRESTORE_CONTEXT();
}

void SerialISR::ISR_Wrapper1(void)
{
	portSAVE_CONTEXT();

    SerialISR::Instances[1]->ISR_Handler();

	portRESTORE_CONTEXT();
}

/*-----------------------------------------------------------*/

void SerialISR::ISR_Handler( void )
{
signed char cChar;
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;


	/* What caused the interrupt? */
	switch( (*IIR) & serINTERRUPT_SOURCE_MASK )
	{
		case serSOURCE_ERROR :	/* Not handling this, but clear the interrupt. */
								cChar = *LSR;
								break;

		case serSOURCE_THRE	:	/* The THRE is empty.  If there is another
								   character in the Tx queue, send it now. */
								if (xQueueReceiveFromISR(qhTX, &cChar, &xHigherPriorityTaskWoken) == pdTRUE) {
									*THR = cChar;
								}
								else {
									/* There are no further characters 
									   queued to send so we can indicate 
									   that the THRE is available. */
									lTHREEmpty = pdTRUE;
								}
								break;

		case serSOURCE_RX_TIMEOUT :
		case serSOURCE_RX	:	/* A character was received.  Place it in 
								   the queue of received characters. */
								cChar = *RBR;
								xQueueSendFromISR(qhRX, &cChar, &xHigherPriorityTaskWoken);
    counter++;
								break;

		default				:	/* There is nothing to do, leave the ISR. */
								break;
	}

	if (xHigherPriorityTaskWoken) {
		portYIELD_FROM_ISR();
	}

	/* Clear the ISR in the VIC. */
	VICVectAddr = serCLEAR_VIC_INTERRUPT;
}
