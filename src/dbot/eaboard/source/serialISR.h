#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void vSerialISRCreateQueues( unsigned portBASE_TYPE uxQueueLength, xQueueHandle *pxRxedChars, xQueueHandle *pxCharsForTx, long volatile **pplTHREEmptyFlag );

/* UART0 interrupt service routine entry point. */
void vUART_ISR_Wrapper( void ); 

#ifdef __cplusplus
}
#endif
