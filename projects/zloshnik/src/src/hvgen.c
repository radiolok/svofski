#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "util.h"
#include "config.h"


//#define FREQDIV 125
#define FREQDIV 100

#define DEADZONE 5

#define T1_ENABLEMASK   BV3(CS12,CS11,CS10)
#define T1_ENABLEBV     _BV(CS10)               // clk/1

void hvgen_setup() 
{
    // /\ counting mode

    // PWM phase and frequency correct, WGM13=1, WGM12,11,10=0, mode 8
    // TOP = ICR1
    // OC1A clear when up-counting, set when down-counting
    // OC1B set when up-counting, clear when down-counting
    TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(COM1B0);

    // mode 8, clock source without prescaling 
    TCCR1B = _BV(WGM13);

    ICR1 = FREQDIV; // approx 10kHz
    OCR1B = FREQDIV/2 + DEADZONE;//+FREQDIV/5;
    OCR1A = FREQDIV/2 - DEADZONE;//-FREQDIV/5;

    DDRB |= BV2(1,2);
}

void hvgen_enable() 
{
    TCCR1B = (TCCR1B & ~T1_ENABLEMASK) | T1_ENABLEBV;
}

void hvgen_disable()
{
    TCCR1B &= TCCR1B & ~T1_ENABLEMASK;
}
