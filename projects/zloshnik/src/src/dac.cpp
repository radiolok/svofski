#include <avr/io.h>
#include <inttypes.h>

#include "util.h"
#include "config.h"
#include "spi.h"

#include "dac.h"

Dac xyz;

inline static void dacss_on()
{
    PORT_DACSS &= ~DACSS_BV;
}

inline static void dacss_off() 
{
    PORT_DACSS |= DACSS_BV;
}

void xyz_spi_setup() 
{
    // fastest SPI
    SPSR = _BV(SPI2X);
    SPCR = BV4(SPE, MSTR, CPHA, CPOL);
}

static void xyz_spi(uint8_t hb, uint8_t lb)
{
    SPDR = hb; spi_wait();
    SPDR = lb; spi_wait();
}

void Dac::Setup(void)
{
    xyz_spi_setup();

    PORT_ZDAC &= ~ZDAC_BV;
    DDR_ZDAC |= ZDAC_BV;

    DDR_DACSS |= DACSS_BV;

    // setup the DAC to use internal 2.048V reference
    // R1:   1      R1:R0 = 11, control reg
    // SPD:  1               1, fast
    // PWR:  0               0, power on
    // R0:   1  
    // xxx
    // REF1: 1      REF1:0 =10, internal 2.048V ref
    // REF0: 0
    dacss_on();
    xyz_spi(0xd0, 0x02);
    dacss_off();
}

void Dac::SetXY(uint8_t dacA, uint8_t dacB) 
{
    dacss_on();
    xyz_spi(0xd0, 0x02);
    dacss_off();


    // A) Write dacB to BUFFER
    // R1:R0    0xx1
    // SPD      x1xx
    // PWR      xx0x
    dacss_on();
    xyz_spi(0x50 | ((dacB >> 4) & 0x0f), (dacB << 4) & 0xf0);
    dacss_off();

    // B) Write dacA to DAC_A and BUFFER to DAC_B
    // R1: R0   1xx0
    // SPD      x1xx
    // PWR      xx0x
    dacss_on();
    xyz_spi(0xc0 | ((dacA >> 4) & 0x0f), (dacA << 4) & 0xf0);
    dacss_off();
}

void Dac::SetZ(uint8_t z) { }

