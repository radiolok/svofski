#include <avr/io.h>

#include "config.h"
#include "spi.h"

void spi_setup() 
{
    DDR_MOSI |= MOSIBV;
    DDR_SCK  |= SCKBV;
    DDR_MISO  &= ~MISOBV;
}

void spi_send(uint8_t b) 
{
    SPDR = b;
    spi_wait();
}
