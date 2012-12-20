#include "Serial.h"
#include "msp430fr5739.h"

Serial com;

Serial::Serial()
{
	// Configure UART pins P2.0 & P2.1
	P2SEL1 |= BIT0 + BIT1;
	P2SEL0 &= ~(BIT0 + BIT1);
	
	// Configure UART 0
	UCA0CTL1 |= UCSWRST; 
	UCA0CTL1 = UCSSEL_2;                      	// Set SMCLK as UCLk 
	UCA0BRW = 8000000/SERIAL_BAUDRATE;			// set baudrate
	UCA0MCTLW = 0x80; 							// set ?
	UCA0CTL1 &= ~UCSWRST;                     	// release from reset
	
	head = tail = 0;
	UCA0IE = UCRXIE;							// enable interrupt
}

void Serial::receive() 
{
	rxbuf[head] = UCA0RXBUF;
	head = ++head == RXBUF_SIZE ? 0 : head;
}

void Serial::putchar(char c) {
  	while (!(UCA0IFG&UCTXIFG));
    UCA0TXBUF = c;
}

int Serial::getchar() {
	int result;
	
	if (!avail()) return -1;
	
	result = rxbuf[tail];
	tail = ++tail == RXBUF_SIZE ? 0 : tail;
	
	return result;
}

void Serial::puts(const char* s) {
  	for (; *s != 0; s++) {
  		putchar(*s);
  	}
}

#pragma vector = USCI_A0_VECTOR
__interrupt void Serial_ISR(void)
{
	com.receive();
	__bic_SR_register_on_exit(LPM2_bits);// Exit LPM4
}

