///\file main.c
///\author Viacheslav Slavinsky
///
///\brief Strobeshnik
///
/// \mainpage Strobeshnik strobes digits in a spinning disc to display time

/// \section Files
/// - main.c    main file
/// - timekeep.c non-RTC-based timekeeping stuff
///

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <stdlib.h>

#include <util/delay.h>
#include <util/atomic.h>

#include "usrat.h"
#include "util.h"
#include "buttonry.h"
#include "modes.h"
#include "timekeep.h"

#define atomic(x) { cli(); x; sei(); }

volatile union _timebcd {
    uint16_t time;
    struct _hhmm {
        uint8_t mm;
        uint8_t hh;
    } hhmm;
} time;

volatile uint8_t blinktick;

//#define TIMERCOUNT 98
#define TIMERCOUNT 104
#define SPINUP_TIME 2048+256
#define MOTORDUTY_LONG   200
#define MOTORDUTY_MIDDLE 60
#define MOTORDUTY_END    12

volatile uint8_t motorcoils = 1;

/// Init display-related DDRs.
void initdisplay() {
    DDRSTROBE |= BV5(0,1,2,3,4);
    PORTMOTOR &= ~BV3(MOT1,MOT2,MOT3);
    DDRMOTOR |= BV3(MOT1,MOT2,MOT3);
}

void extint_init() {
    DDRD &= ~BV2(2,3);
    EICRA = _BV(ISC01); // falling edge of INT0 
    EIMSK = _BV(INT0);  // enable INT0
}

/// Return current BCD display value 
inline uint16_t get_display_value() {
    return time.time;
}

/// Start timer 0.
void timer0_init() {
    TIMSK0 |= _BV(TOIE0);    // enable Timer0 overflow interrupt
    TCNT0 = 256-TIMERCOUNT;
    TCCR0B = _BV(CS01);     // 20MHz/8

}

#define TICKS_PER_SECOND 23200
#define DIGITTIME 64

uint16_t counter1 = 0xfff;
uint16_t counter2 = 0x1fff;

int16_t phase1 = 174-384, phase2 = 125-384, phase3 = 88-384, phase4 = 49-384, phase5 = 0-384;
struct _phase_precalc {
    int16_t p1, p2, p3, p4, p5;
} phasepre;

int16_t strobectr = -768/2;
int16_t strobeindexmark = 32;

uint16_t spintime = SPINUP_TIME;
uint16_t spinctr = SPINUP_TIME;

uint8_t motorduty = 0;
volatile uint8_t eightctr = 0;

uint8_t motorduty_set; 

uint16_t strobe_fullspin = 768;
uint16_t strobe_halfspin = 384;

volatile uint16_t time_ctr = 0;
volatile uint16_t blinkctr = TICKS_PER_SECOND/4;


enum _spinup {
    SPIN_START = 0,
    SPIN_SPUN,
    SPIN_DUTYDOWN,
    SPIN_STABLE,
    SPIN_STOP,
};
uint8_t spinned_up;

uint8_t motorbits = 1;

uint8_t halfctr;

ISR(TIMER0_OVF_vect) {
    uint8_t yes;
    
    TCNT0 = 256-TIMERCOUNT;

    yes = PORTSTROBE & ~BV5(0,1,2,3,4);

    do {
        if (blinkmode_get() == BLINK_ALL && ((blinktick & _BV(1)) == 0)) break;
        
        if (blinkmode_get() != BLINK_HH || (blinktick & _BV(1))) {
            if (abs(strobectr - phasepre.p1) < 2) {
                yes |= _BV(0);
            } 
            
            if (abs(strobectr - phasepre.p2) < 2) {
                yes |= _BV(1);
            } 
        }
    
        if (blinkmode_get() != BLINK_MM || (blinktick & _BV(1))) {
            if (abs(strobectr - phasepre.p4) < 2) {
                yes |= _BV(3);
            } 
            
            if (abs(strobectr - phasepre.p5) < 2) {
                yes |= _BV(4);
            } 
        }
    } while (0);

    if (dotmode != DOT_OFF && ((blinktick & _BV(1)) || dotmode == DOT_ON) && abs(strobectr - phasepre.p3) < 2) {
        yes |= _BV(2);
    }
    
    PORTSTROBE = yes;

    
    if (--spinctr == 0) {
        spinctr = spintime;
        motorbits <<= 1;
        if (motorbits == 8) {
            motorbits = 1;
        }

        motorduty = spintime > 128 ? MOTORDUTY_LONG : motorduty_set;
    }
    
    yes = PORTMOTOR & ~BV3(MOT1,MOT2,MOT3);
    if (spinned_up != SPIN_STOP) {
        if (motorduty > 0) {
            --motorduty;
            yes |= motorbits;
        }
    }

    PORTMOTOR = yes;
    
    strobectr++;
    if (strobectr == strobe_halfspin) {
        strobectr = -strobe_halfspin;
    }
    
    if (counter1 > 0) --counter1;
    if (counter2 > 0) --counter2;
    if (blinkctr > 0) --blinkctr;
    
    if (--time_ctr == 0) {
        time_ctr = TICKS_PER_SECOND;// 25511
        blinktick |= _BV(0);
    } 
    
}

void update_parameters() {
    if (counter1 == 0) {
        atomic(counter1 = 0xfff);
        if (spinned_up == SPIN_START) {
            if (spintime > 352+512) {
                atomic(spintime -= 64);
            } else
            if (spintime > 352 + 256) { 
                atomic(spintime -= 32);
            } else
            if (spintime > 96) {
                atomic(spintime -= 8);
            } else {
                atomic(spintime-=2);
            }
            if (spintime == 64) {
                atomic(spinned_up = SPIN_SPUN);
            }
        }
    }
    
    if (counter2 == 0) {
        atomic(counter2 = 0x7ff);
        switch (spinned_up) {
            case SPIN_START:
                break;
            case SPIN_SPUN:
                atomic(spinned_up = SPIN_DUTYDOWN);
                break;
            case SPIN_DUTYDOWN:
                atomic(motorduty_set--);
                if (motorduty_set == MOTORDUTY_END) {
                    atomic(spinned_up = SPIN_STABLE);
                }
                break;
            case SPIN_STABLE:
                break;
        }
        
    }
}

int8_t indexctr = 0;
int8_t rps = 0;

ISR(INT0_vect) {
    int16_t error = strobectr - strobeindexmark;
    if (abs(error) > 1) {
        strobe_fullspin = 768+(error < 0 ? -2 : 2);  // seek
        strobe_halfspin = strobe_fullspin/2;
    } else {
        strobe_fullspin = 768;
        strobe_halfspin = 384;
    }
    indexctr++;
}

uint8_t stall_detect() {
    rps = indexctr;
    
    if ((spinned_up == SPIN_STABLE && indexctr < 20) || (spintime < 1024 && indexctr < 2)) {
        printf_P(PSTR("i=%d\n"), indexctr);
        return 1;
    } 
    
    atomic(indexctr = 0);
    return 0;
}

/// Start fading time to given value.
/// Transition is performed in TIMER0_OVF_vect and takes FADETIME cycles.
void fadeto(uint16_t t) {
    time.time = t;
    phasepre.p1 = phase1 + ((((time.hhmm.hh&0xf0)>>4))<<6);
    phasepre.p2 = phase2 + ((((time.hhmm.hh&0x0f)>>0))<<6);
    phasepre.p3 = phase3 + (10<<6);
    phasepre.p4 = phase4 + ((((time.hhmm.mm&0xf0)>>4))<<6);
    phasepre.p5 = phase5 + ((((time.hhmm.mm&0x0f)>>0))<<6);  
}

void spinup_setup() {
    spinned_up = SPIN_START;
    spintime = SPINUP_TIME;
    spinctr = SPINUP_TIME;
    motorduty_set = MOTORDUTY_MIDDLE;
    
    atomic(motorbits = 0);
    PORTSTROBE &= ~BV5(0,1,2,3,4);
    _delay_ms(500);
}

void spin_enable() {
    spinned_up = SPIN_START;
    atomic(motorbits = 1);
}

void spin_disable() {
    spinned_up = SPIN_STOP;
    atomic(motorbits = 0);
}

/// Program main
int main() {
    uint8_t i;
    uint8_t byte;
    volatile uint16_t skip = 0;
    uint8_t uart_enabled = 0;

    usart_init(F_CPU/16/19200-1);
    
    printf_P(PSTR("\033[2J\033[HB%s CAN WE MAKE IT BACK TO EARTH? %02x\n"), BUILDNUM, MCUCR);


    spinup_setup();
    
    sei();

    timer0_init();

    initdisplay();
    dotmode_set(DOT_BLINK);
    buttons_init();
    extint_init();

    spin_enable();
    
    for(i = 0;;i++) {
        wdt_reset();
        
        // handle keyboard commands
        if (uart_available()) {
            byte = uart_getchar();
            switch (uart_enabled) {
                case 0: if (byte == 'z') 
                            uart_enabled = 1;
                        else
                            uart_enabled = 0;
                        break;
                case 1: if (byte == 'c') 
                            uart_enabled = 2;
                        else
                            uart_enabled = 0;
                        break;
                case 2:
                        switch (byte) { 
                        case 'd':   phase1--; break;
                        case 'D':   phase1++; break;
                        case 'f':   phase2--; break;
                        case 'F':   phase2++; break;
                        case 'g':   phase3--; break;
                        case 'G':   phase3++; break;
                        case 'h':   phase4--; break;
                        case 'H':   phase4++; break;
                        case 'j':   phase5--; break;
                        case 'J':   phase5++; break;
                        case 'a':   strobe_fullspin--;
                                    break;
                        case 's':   strobe_fullspin++;
                                    break;
                        case 'e':   motorduty_set--;
                                    break;
                        case 'r':   motorduty_set++;
                                    break;
                        case '1':   strobeindexmark--;
                                    break;
                        case '2':   strobeindexmark++;
                                    break;
                        case 't':   time.time = 0;
                                    break;
                        case '.':   break;
                        default:
                                    break;
                        }
            
                        if (byte >= '0' && byte <= '9') {
                            byte = byte - '0';
                            fadeto((byte<<12)+(byte<<8)+(byte<<4)+byte);
                            skip = 255;
                        }
                        
                        printf_P(PSTR("time=%04x ph=%d,%d,%d,%d,%d fullspin=%d duty=%d strobeinexmark=%d spintime=%d spinned_up=%d\n"), 
                            time, phase1,phase2,phase3,phase4,phase5, 
                            strobe_fullspin, motorduty_set, strobeindexmark,
                            spintime, spinned_up
                            );
                        break;
            }
        }

        update_parameters();

        buttonry_tick();

        if (blinktick & _BV(0) != 0) {
            blinktick &= ~_BV(0);

            time_nextsecond();
            if (stall_detect()) {
                spinup_setup();
                spin_disable();
                printf_P(PSTR("STALL\n"));
            }
            
            if (rps == 0 && spinned_up == SPIN_STOP) {
                printf_P(PSTR("RESTART\n"));
                spin_enable();
            }
        }
        
        switch (mode_get()) {
            case HHMM:
                fadeto(time_get_hhmm());
                break;
            default:
            case MMSS:
                fadeto(time_get_mmss());
                break;
        }

        if (blinkctr == 0) {
            cli();
            blinkctr = TICKS_PER_SECOND/4;
            sei();
            if (blinkhandler != NULL) {
                blinkhandler(1);
            }
        }

    
        if (time_ctr > TICKS_PER_SECOND/2) {
            blinktick |= _BV(1);
        } else {
            blinktick &= ~_BV(1);
        }
        
        _delay_ms(100);

        // just waste time
        //while((blinktick & _BV(2)) == 0) {
        //    sleep_enable();
        //    sleep_cpu();
        //}
    }
}

