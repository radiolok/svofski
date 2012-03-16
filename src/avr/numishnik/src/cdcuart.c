/* cdcuart.c
 * based on uart.c, see comment block below 
 */ 

/* Name: uart.c
 * Project: AVR USB driver for CDC interface on Low-Speed USB
 * Author: Osamu Tamura
 * Creation Date: 2006-06-18
 * Tabsize: 4
 * Copyright: (c) 2006 by Recursion Co., Ltd.
 * License: Proprietary, free under certain conditions. See Documentation.
 *
 * 2006-07-08   adapted to higher baudrate by T.Kitazawa
 */
/*
General Description:
    This module implements the UART rx/tx system of the USB-CDC driver.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>   /* needed by usbdrv.h */
#include "usbdrv.h"
#include "cdcuart.h"

extern uchar    sendEmptyFrame;

/* UART buffer */
uchar    urptr, uwptr, irptr, iwptr;
uchar    rx_buf[RX_SIZE+HW_CDC_BULK_IN_SIZE], tx_buf[TX_SIZE];

int cdc_dsr(void) 
{
    return uwptr != irptr;
}

int cdc_getchar(void) 
{
    int result;

    if (cdc_dsr()) {
        result = tx_buf[irptr];
        irptr = (irptr + 1) & TX_MASK;

        if( usbAllRequestsAreDisabled() && uartTxBytesFree()>HW_CDC_BULK_OUT_SIZE ) {
            usbEnableAllRequests();
        }
    } else {
        result = -1;
    }

    return result;
}

int cdc_rts() 
{
    return ((iwptr+1) & RX_MASK) != urptr;
}

static void cdc_puu(char c) {
    int next = (iwptr+1) & RX_MASK;
    if( next!=urptr ) {
        rx_buf[iwptr] = c;
        iwptr = next;
    }
 }

int cdc_putchar(char c, FILE* f) 
{
    cdc_puu(c);
    if (c == '\n') {
        cdc_puu('\r');
    }

    return 0;
}

void uartPoll(void)
{
	uchar		next;

#if 0
#error see cdc_dsr() and cdc_getchar()

	/*  device => RS-232C  */
	while( (UCSR0A&(1<<UDRE0)) && uwptr!=irptr && (UART_CTRL_PIN&(1<<UART_CTRL_CTS)) ) {
        UDR0    = tx_buf[irptr];
        irptr   = (irptr+1) & TX_MASK;

        if( usbAllRequestsAreDisabled() && uartTxBytesFree()>HW_CDC_BULK_OUT_SIZE ) {
            usbEnableAllRequests();
        }
    }
#endif

#if 0
#error see cdc_putchar()

	/*  device <= RS-232C  */
	while( UCSR0A&(1<<RXC0) ) {
	    next = (iwptr+1) & RX_MASK;
		if( next!=urptr ) {
	        uchar   status, data;

	        status  = UCSR0A;
	        data    = UDR0;
	        status  &= (1<<FE0) | (1<<DOR0) | (1<<UPE0);
	        if(status == 0) { /* no receiver error occurred */
	            rx_buf[iwptr] = data;
	            iwptr = next;
	        }
		}
		else {
			UART_CTRL_PORT	&= ~(1<<UART_CTRL_RTS);
			break;
		}
    }
#endif

	/*  USB <= device  */
    if( usbInterruptIsReady() && (iwptr!=urptr || sendEmptyFrame) ) {
        uchar   bytesRead, i;

        bytesRead = (iwptr-urptr) & RX_MASK;
        if(bytesRead>HW_CDC_BULK_IN_SIZE)
            bytesRead = HW_CDC_BULK_IN_SIZE;
		next	= urptr + bytesRead;
		if( next>=RX_SIZE ) {
			next &= RX_MASK;
			for( i=0; i<next; i++ )
				rx_buf[RX_SIZE+i]	= rx_buf[i];
		}
        usbSetInterrupt(rx_buf+urptr, bytesRead);
        urptr   = next;
		//if( bytesRead )
	    //		UART_CTRL_PORT	|= (1<<UART_CTRL_RTS);

        /* send an empty block after last data block to indicate transfer end */
        sendEmptyFrame = (bytesRead==HW_CDC_BULK_IN_SIZE && iwptr==urptr)? 1:0;
    }
}

