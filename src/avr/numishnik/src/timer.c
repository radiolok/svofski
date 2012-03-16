#include <avr/io.h>
#include <avr/interrupt.h>

#include "util.h"
#include "globals.h"

#define TIMERCOUNT 25


/// Start timer 0. Timer0 runs at 46875 KHz
/// The speed is dictated by the need to keep the neon dot ionized at all times
void timer0_init() {
    TIMSK |= _BV(TOIE0);    // enable Timer0 overflow interrupt
    TCNT0 = 256-TIMERCOUNT;
    TCCR0 = _BV(CS02);

    blinkdot = 0;
    blinkcounter = 1;
}

ISR(TIMER0_OVF_vect, ISR_NOBLOCK) {
    TCNT0 = 256-TIMERCOUNT;

    --blinkcounter;

    if (blinkcounter == 1875/2) 
        blinkdot = 1; //(blinkcounter & 7) == 0 ? 1 : 0;
    else if (blinkcounter == 0) {
        blinkcounter = 1875;
        blinkdot = 0;

        uint8_t m = bcd_increment(the_time.minute);
        uint8_t h = the_time.hour;
        if (m == 0x60) {
            m = 0;
            h = bcd_increment(the_time.hour);
            if (h == 0x24) {
                h = 0;
                m = 0;
            }
        }
        the_time.hour = h;
        the_time.minute = m;
    }

}
