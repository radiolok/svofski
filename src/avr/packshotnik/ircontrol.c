//! Generate modulated pulses to simulate Canon RC-1 remote
//! 
//! Pulse-width encoding:
//! One  is 550us on, 5250us off
//! Zero is 550us on, 7300us off
//! Trailing pulse is 550us on
//! Gap is 100000us
//! Only one bit
//! Allegedly, 32768Hz carrier freq

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <stdlib.h>

#include "ircontrol.h"

#define CARRIER_FREQ    (8000000/244)  // 32787Hz
#define ON_COUNT        32
#define OFF_COUNT       480

static volatile uint8_t repeats;
static volatile uint8_t retries;
static volatile uint16_t count;
static volatile uint8_t state;

inline void carrier(uint8_t on) {
    TCCR1A = on ? _BV(COM1A0) : 0;               // toggle OC1A on compare match
}

void irc_init() {
    TCCR1B = _BV(WGM12) | _BV(CS10);    // CTC1: OCR1A = TOP
    OCR1A = 126;    // ~= 32787 hz
    DDRB |= _BV(1);
    PORTB &= ~_BV(1);
}

// turn on for 550us    (18 counts)
// turn off for 7300us  (239 counts)
// turn on for 550us
// repeat 16 times
void irc_shutter() {
    TIMSK |= _BV(OCIE1A);
    state = 2;
    count = ON_COUNT;
    repeats = 2;
    retries = 1;
}

ISR(TIMER1_COMPA_vect) {
    if (count > 0) {
        count--;
        if (count == 0) {
            switch (state) {
                case 1:
                    state = 2;
                    carrier(0);
                    count = OFF_COUNT;
                    break;
                case 2:
                    if (repeats-- > 0) {
                        state = 1;
                        carrier(1);
                        count = ON_COUNT;
                    } else {
                        if (retries == 0) {
                            state = 0;
                            carrier(0);
                            TIMSK &= ~_BV(OCIE1A);
                        } else {
                            retries--;
                            count = 65000;
                            carrier(0);
                            state = 3;
                        }
                    }
                    break;
                case 3:
                    // fire again
                    state = 2;
                    repeats = 2;
                    break;
                default:
                    break;
            }
        }
    }
}

