#include <inttypes.h>
#include <avr/io.h>

#include "util.h"

#include "lobo.h"

static volatile uint8_t pulsecount;

void lobo_init() {
    DDRD &= _BV(2);     // PORTD.2 is input, INT0
    
    //MCUCR |= BV2(ISC01,ISC00);  // rising edge generates request
    MCUCR |= _BV(ISC00);  // any logical change generates request
    GICR |= _BV(INT0);
    
    D_LOBO |= _BV(BMOTOR);
    lobo_run(0);
}

void lobo_run(uint8_t on) {
    if (on) {
        P_LOBO |= _BV(BMOTOR);
    } else {
        P_LOBO &= ~_BV(BMOTOR);
    } 
}

uint8_t lobo_is_running() {
    return (P_LOBO & _BV(BMOTOR)) != 0;
}

uint8_t lobo_get_pulsecount() {
    return pulsecount;
}

void lobo_reset_pulsecount() {
    pulsecount = 0;
}

//! Lobo feedback loop
void lobo_index() {
    pulsecount++;
}
