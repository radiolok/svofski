#include <inttypes.h>

#include "msp430fr5739.h"
#include "I2CMaster.h"

I2CMaster i2c;

I2CMaster::I2CMaster()
{
}

volatile uint16_t ucb0ifg;
volatile uint16_t ucb0ctlw0;
volatile uint16_t stt;

void I2CMaster::Init() {
    // enter reset
    UCB0CTLW0 |=  UCSWRST;
    
    UCB0CTLW0 |= UCSSEL_2 + UCMODE_3 + UCMST;      // I2C modo, maestro, SMCLK clock source
    UCB0BRW = 72;//64;
    // no automatic STOP assertion
    //UCB0CTLW1 = UCASTP_2;               // automatic STOP assertion
    //UCB0TBCNT = 7;                      // TX 7 bytes of data?
    
    // P1.7 = SCL
    // P1.6 = SDA
    P1SEL1 |= 0300;                       // pins!!!
    P1SEL0 &= 0077;

    // UCB0IE |= UCTXIE;                   // interrupts

    // exit reset
    UCB0CTLW0 &= ~UCSWRST;
    ucb0ctlw0 = UCB0CTLW0;
}


int I2CMaster::Start(int address)
{
    UCB0I2CSA = address;

    UCB0CTLW0 |= UCTR + UCTXSTT;             // UCTR=i2c tx mode, UCTXSTT = start condition
                                            // UCTXSTT is cleared automatically after slave
                                            // address is transmitted
                                 
    stt = UCB0CTLW0;
    //while(UCB0CTL1 & UCTXSTT);            // UCTXSTT is cleared after address is sent

    while(((ucb0ifg=UCB0IFG) & UCTXIFG0) == 0) ucb0ctlw0 = UCB0CTLW0;       // UCTXIFG0 is set as soon 
                                            // as transmission is possible    
                                        
    if (UCB0IFG & UCNACKIFG) return -1;     // slave did not acknowledge address

    return 0;
}

void I2CMaster::Send(uint8_t data)
{
    // write data to buffer
    UCB0TXBUF = data;
    // wait until data are copied to shift register
    while((UCB0IFG & UCTXIFG0) == 0);
}

void I2CMaster::Request(uint8_t address) 
{
    // receive mode, (re)start condition
    UCB0CTLW0 &= ~UCTR;
    UCB0CTLW0 |= UCTXSTT;
    
    // UCTXSTT is cleared after address is sent
    while(UCB0CTL1 & UCTXSTT);       
}

int I2CMaster::Receive()
{
    // check for NAK
    if (UCB0IFG &  UCNACKIFG) return -1;
    while((UCB0IFG & UCRXIFG0) == 0);   // wait until data are received
    
    return UCB0RXBUF;
}

int I2CMaster::End()
{
    // set stop condition 
    UCB0CTLW0 |= UCTXSTP;        
    
    // wait until the end to be sure
    while(UCB0CTLW0 & UCTXSTP);

    return 0;
}

