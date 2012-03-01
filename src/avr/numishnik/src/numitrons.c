

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include "numitrons.h"

volatile uint8_t numitrons_blank = 0;
volatile uint8_t blinkdot = 0;

const PROGMEM uint8_t number2segment[16] = {NUMI0, NUMI1, NUMI2, NUMI3, NUMI4,
                                            NUMI5, NUMI6, NUMI7, NUMI8, NUMI9,
                                            NUMIA, NUMIB, NUMIC, NUMID, NUMIE, NUMIF};


void numitronsInit()
{
    DDRB |= BV2(3,5);   // MOSI, SCK outputs
    DDRB &= ~_BV(4);    // MISO input
    SPCR = BV5(DORD, SPE, MSTR, CPHA, SPR1);
    SPCR |= _BV(DORD);
    SPCR &= ~_BV(CPHA);
    DDRC |= _BV(0);     // Latch Enable for Macroblocks, output
    PORTC &= ~_BV(0);   // LE = 0, disable
}

static inline void spi_wait() {
    while (!(SPSR & _BV(SPIF)));
}


void numitronsBCD(uint16_t num)
{
    numitronsInit();

    SPDR = blinkdot |
           ((numitrons_blank & 1) ? pgm_read_byte(&number2segment[017 & num]) : 0);
    spi_wait();
    SPDR = (numitrons_blank & 2) ? pgm_read_byte(&number2segment[017 & (num>>4)]) : 0;
    spi_wait();
    SPDR = (numitrons_blank & 4) ? pgm_read_byte(&number2segment[017 & (num>>8)]) : 0;
    spi_wait();
    SPDR = (numitrons_blank & 8) ? pgm_read_byte(&number2segment[017 & (num>>12)]) : 0;
    spi_wait();
    PORTC |= _BV(0);   // latch le data
    asm __volatile__ ("nop");
    asm __volatile__ ("nop");
    asm __volatile__ ("nop");
    PORTC &= ~_BV(0);   // LE = 0
}

