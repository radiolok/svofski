#include <avr/io.h>

#include "spi.h"

inline void spi_wait() 
{
    while (!(SPSR & _BV(SPIF)));
}

void spi_send(uint8_t b) 
{
    SPDR = b;
    spi_wait();
}
