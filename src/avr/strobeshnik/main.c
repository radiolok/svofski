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

#define TICKS_PER_SECOND 25         //!< for timekeeping, Timer1 ticks 25 times per second

#define BLINKCTR_LOAD    6          //!< Blink counter expires approx 4 times per second

uint8_t counter1 = 0;

#define SPINTIME_CRUISE 70// 80//90 /// ~ 355hz coil switching rate

#define STROBE_FULLSPIN (SPINTIME_CRUISE*48) //(4322)
#define STROBE_HALFSPIN (STROBE_FULLSPIN/2)

#define DIGIT_SPAN (STROBE_FULLSPIN/12)

int16_t strobectr = -STROBE_FULLSPIN/2;
int16_t strobeindexmark = 32 * 2;

int16_t measure_ctr = 0;
int16_t measured_fullspin = 0;

uint16_t strobe_fullspin = STROBE_FULLSPIN;
uint16_t strobe_halfspin = STROBE_HALFSPIN;

// precalculated phases of every character position

#if 0 // for SPINTIME_CRUISE = 90
int16_t phase1 = 2930,//2*174-STROBE_HALFSPIN, 
        phase2 = 2630, 
        phase3 = 2930, 
        phase4 = 2250, 
        phase5 = 1970;
#endif

int16_t phase1 = 2290,
        phase2 = 2070, 
        phase3 = 710, 
        phase4 = 1730, 
        phase5 = 1530;


// precalculated absolute phase values for characters at their places
struct _phase_precalc {
    int16_t p1, p2, p3, p4, p5;
} phasepre;



volatile uint8_t time_ctr = 1;

volatile uint8_t blinkctr = BLINKCTR_LOAD; //!< blinkctr == 0 every 1/5th of a second

volatile uint8_t blinktick;         //!< bit flags for sync

#define TIMERCOUNT 190 //160              /// = 125kHz
#define SPINUP_TIME 1024            //!< initial spintime

uint16_t spintime = SPINUP_TIME;    //!< this many counts between coil rotation
uint16_t spinctr = SPINUP_TIME;     //!< current counter

uint8_t timercount = TIMERCOUNT;    //!< this many timer ticks per count: global scale can be changed by this

int16_t error = 0;                    //!< phase error between current and needed index mark position

enum _spinup {
    SPIN_START = 0,
    SPIN_SPUN,
    SPIN_DUTYDOWN,
    SPIN_PRESTABLE,
    SPIN_STABLE,
    SPIN_STOP,
};
uint8_t spinned_up;                 //!< spinup state 

#define MOTORBITS_INIT 3

uint8_t motorbits = MOTORBITS_INIT;              //!< motor coils: 001->010->100

int8_t indexctr = 0;                //!< index ticks count
int8_t rps = 0;                     //!< rotations per blinktick (quarter-second)

#define atomic(x) { cli(); x; sei(); }

volatile union _timebcd {
    uint16_t time;
    struct _hhmm {
        uint8_t mm;
        uint8_t hh;
    } hhmm;
} time;

/// Init display-related DDRs.
void initdisplay() {
    DDRSTROBE |= BV5(0,1,2,3,4);
    PORTMOTOR &= ~BV3(MOT1,MOT2,MOT3);
    DDRMOTOR |= BV3(MOT1,MOT2,MOT3);
}

void extint_init() {
    DDRD &= _BV(2);
    EICRA = _BV(ISC01); // falling edge of INT0 
    //EIMSK = _BV(INT0);  // enable INT0
}

/// Return current BCD display value 
inline uint16_t get_display_value() {
    return time.time;
}

/// Start timer 0
void timer0_init() {
    TIMSK0 |= _BV(TOIE0);    // enable Timer0 overflow interrupt
    TCNT0 = 256 - TIMERCOUNT;
    //TCCR0B = _BV(CS01);     // 20MHz/8
    TCCR0B = _BV(CS00);     // 20MHz
}

/// Start timer 1
void timer1_init() {
    TIMSK1 |= _BV(TOIE1);
    TCNT1 = 65536-3125;
    TCCR1B = _BV(CS12);     // 20MHz/256 -> /3125 => 25 ticks/s
}


/// Update time-related counters
ISR(TIMER1_OVF_vect, ISR_NOBLOCK) {
    TCNT1 = 65536-3125;
    if (--time_ctr == 0) {
        time_ctr = TICKS_PER_SECOND;// 25511
        blinktick |= _BV(0);
    } 

    if (counter1 > 0) --counter1;
    if (blinkctr > 0) --blinkctr;
}

/// 00 00 11 -> 001
/// 00 01 10 -> 011
/// 00 11 00 -> 010 etc
// 1 00 00 00 
inline uint8_t to_coils(const uint8_t wide) {
    return ((wide & 0x03) ? 1 : 0) |
           ((wide & 0x0c) ? 2 : 0) |
           ((wide & 0x30) ? 4 : 0);
}


#define BITSPREAD_5_OF_5 0x1f
#define BITSPREAD_2_OF_5 0x0a

#define BITSPREAD_4_OF_8    0x55
#define BITSPREAD_6_OF_8    0xca
#define BITSPREAD_8_OF_8    0xff
#define BITSPREAD_2_OF_8    0x44 

#define BITSPREAD_POKE   BITSPREAD_6_OF_8
#define BITSPREAD_TUG    BITSPREAD_4_OF_8
#define BITSPREAD_PRECRUISE BITSPREAD_4_OF_8
#define BITSPREAD_CRUISE BITSPREAD_2_OF_8

#define BITSPREAD_WIDTH 8
uint8_t bitspread_reg = BITSPREAD_TUG;

uint8_t timer0_div = 8;

#define ONTIME 12

ISR(TIMER0_OVF_vect) {
    uint8_t yes;
    
    TCNT0 = 256 - TIMERCOUNT;

    // update index: poll instead of interrupt
    if (EIFR & _BV(INTF0)) {
        EIFR |= _BV(INTF0);
        measured_fullspin = measure_ctr;
        measure_ctr = 0;

        error = abs(measured_fullspin - STROBE_FULLSPIN);
        indexctr++;
    }

///---
    yes = PORTMOTOR & ~BV3(MOT1,MOT2,MOT3);
    
    if (spinned_up != SPIN_STOP) {
        if (bitspread_reg & 0x40) {
            bitspread_reg = 0x1 | (bitspread_reg << 1);
            yes |= to_coils(motorbits);
        } else {
            bitspread_reg = bitspread_reg << 1;
        }
    }
    
    PORTMOTOR = yes;

    // cycle the coils
    if (--spinctr == 0) {
        spinctr = spintime;
        motorbits <<= 1;
        if ((motorbits & 0x40) != 0) {
            motorbits |= 1;
            motorbits &= 0x3f;
        }
    }
    measure_ctr++;
    //return;

    // strobe the lights
    yes = PORTSTROBE & ~BV5(0,1,2,3,4);

    do {
        //if (blinkmode_get() == BLINK_ALL && ((blinktick & _BV(1)) == 0)) break;
        
        //if (blinkmode_get() != BLINK_HH || (blinktick & _BV(1))) {
            int diff = measure_ctr - phasepre.p1;
            if (diff > -ONTIME && diff < ONTIME) {
                yes |= _BV(0);
            } 
            
            diff = measure_ctr - phasepre.p2;
            if (diff > -ONTIME && diff < ONTIME) {
                yes |= _BV(1);
            } 

            diff = measure_ctr - phasepre.p4;
            if (diff > -ONTIME && diff < ONTIME) {
                yes |= _BV(3);
            } 
            
            diff = measure_ctr - phasepre.p5;
            if (diff > -ONTIME && diff < ONTIME) {
                yes |= _BV(4);
            } 
        //}
    } while (0);
    int diff = measure_ctr - phasepre.p3;
    //if (dotmode != DOT_OFF && ((blinktick & _BV(1)) || dotmode == DOT_ON) && diff > -ONTIME && diff < ONTIME) {
    if ((blinktick & _BV(1)) && diff > -ONTIME && diff < ONTIME) {
        yes |= _BV(2);
    }
    
    PORTSTROBE = yes;
    // ---------------
}


/// Initialize spinup variables
void spinup_setup() {
    spinned_up = SPIN_START;
    spintime = SPINUP_TIME << 1;
    spinctr = SPINUP_TIME << 1;
    
    atomic(motorbits = 0);
    PORTSTROBE &= ~BV5(0,1,2,3,4);
    _delay_ms(500);
}


/// Enable spinup
void spin_enable() {
    spinned_up = SPIN_START;
    atomic(motorbits = MOTORBITS_INIT);
    atomic(bitspread_reg = BITSPREAD_POKE);
}


/// Stop spinning
void spin_disable() {
    spinned_up = SPIN_STOP;
    atomic(motorbits = 0);
}

void respin() {
    spinup_setup();
    spin_enable();
}

uint8_t pre_stable = 0;

/// Spinup control. Open-loop except for rotation index mark feedback.
/// Start slowly, then if the platter seems to be spinning, switch
/// into more aggressive mode, reach up top speed slower, then
/// reduce PWM rate down to DUTY_STABLE.
void update_parameters() {
    if (counter1 == 0) {
        atomic(counter1 = 2);

        uint16_t spintime_out = spintime >> 1;

        switch (spinned_up) {
        case SPIN_START:
            printf_P(PSTR("%d "), indexctr);
            spintime_out += 2;
            if (spintime_out > SPINUP_TIME + 64) {
                respin();
            }
            else if (indexctr > 3) {
                atomic(bitspread_reg = BITSPREAD_TUG);
                spinned_up = SPIN_SPUN;
            }
            break;
        case SPIN_SPUN:
            if (spintime_out > 256) {
                spintime_out -= 64;
            } else {
                spintime_out -= 4;
            }
            if (spintime_out < SPINUP_TIME/8 && rps == 0) {
                printf_P(PSTR("RESPIN %d\n"), spintime_out);
                respin();
            }
            
            if (spintime_out <= SPINTIME_CRUISE) {
                spintime_out = SPINTIME_CRUISE;
                spinned_up = SPIN_DUTYDOWN;
            }
            break;
        case SPIN_DUTYDOWN:
            if (abs(error) < 40) {
                atomic(if (motorbits == 3) {
                    motorbits = 1;
                    spinned_up = SPIN_PRESTABLE;
                    bitspread_reg = BITSPREAD_PRECRUISE;
                });
                if (spinned_up == SPIN_PRESTABLE) {
                    printf_P(PSTR("CRUISING\n"));
                    pre_stable = 64;
                }
            }
            break;
        case SPIN_PRESTABLE:
            if (pre_stable > 0) --pre_stable;
            if (pre_stable == 0 && abs(error) < 2) {
                atomic(
                    bitspread_reg = BITSPREAD_CRUISE;
                );
                spinned_up = SPIN_STABLE;
                printf_P(PSTR("STABLE\n"));
            }
            break;
        default:
            // stable spin, do nothing
            
            break;
        }
        if (spintime_out != spintime) {
            atomic(spintime = spintime_out << 1);
        }
    }
}


//! External interrupt. Closed-loop control of disk phase. 
//! If |strobectr-strobeindexmark| < sigma, change full spin count and
//! wait until error becomes negligible.
ISR(INT0_vect) {
    measured_fullspin = measure_ctr;
    measure_ctr = 0;

    error = abs(measured_fullspin - STROBE_FULLSPIN);
    indexctr++;
}

//! Update rps variable and detect stall situation.
//! \returns 1 when stall is detected
uint8_t stall_detect() {
    atomic(rps = indexctr; indexctr = 0);
    
    if ((spinned_up == SPIN_STABLE || spinned_up == SPIN_DUTYDOWN) && rps < 2) {
        printf_P(PSTR("i=%d\n"), rps);
        return 1;
    } 
    
    return 0;
}

/// Start fading time to given value.
/// Transition is performed in TIMER0_OVF_vect and takes FADETIME cycles.
void fadeto(uint16_t t) {
    time.time = t;
    phasepre.p1 = (phase1 + ((((time.hhmm.hh&0xf0)>>4)) * DIGIT_SPAN)) % STROBE_FULLSPIN;
    phasepre.p2 = (phase2 + ((((time.hhmm.hh&0x0f)>>0)) * DIGIT_SPAN)) % STROBE_FULLSPIN;
    phasepre.p3 = phase3 + (10<<6);
    phasepre.p4 = (phase4 + ((((time.hhmm.mm&0xf0)>>4)) * DIGIT_SPAN)) % STROBE_FULLSPIN;
    phasepre.p5 = (phase5 + ((((time.hhmm.mm&0x0f)>>0)) * DIGIT_SPAN)) % STROBE_FULLSPIN;  
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
    timer1_init();

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
                        case 'd':   phase1-=10; break;
                        case 'D':   phase1+=10; break;
                        case 'f':   phase2-=10; break;
                        case 'F':   phase2+=10; break;
                        case 'g':   phase3-=10; break;
                        case 'G':   phase3+=10; break;
                        case 'h':   phase4-=10; break;
                        case 'H':   phase4+=10; break;
                        case 'j':   phase5-=10; break;
                        case 'J':   phase5+=10; break;
                        case 'a':   strobe_fullspin--;
                                    break;
                        case 's':   strobe_fullspin++;
                                    break;
                        case 'e':   OCR2A--;//motorduty_set--;
                                    break;
                        case 'E':   OCR2B--;
                                    break;            
                        case 'r':   OCR2A++;//motorduty_set++;
                                    break;
                        case 'R':   OCR2B++;
                                    break;
                        case '1':   timercount--;
                                    break;
                        case '2':   timercount++;
                                    break;
                        case 't':   time.time = 0;
                                    break;
                        case '.':   break;
                        case '0':   
                                    spinup_setup();
                                    spin_disable();
                                    break;
                        default:
                                    break;
                        }
            
                        if (byte >= '0' && byte <= '9') {
                            byte = byte - '0';
                            fadeto((byte<<12)+(byte<<8)+(byte<<4)+byte);
                            skip = 255;
                        }
                        
                        printf_P(PSTR("timercount=%d ph=%d,%d,%d,%d,%d fullspin=%d strobeinexmark=%d spintime=%d spinned_up=%d rps=%d error=%d measured=%d\n"), 
                            timercount, phase1,phase2,phase3,phase4,phase5, 
                            strobe_fullspin, strobeindexmark,
                            spintime, spinned_up, rps, error, measured_fullspin
                            );
                        break;
            }
        }

        update_parameters();
        // if (spinned_up != SPIN_STABLE) {
        //     printf_P(PSTR("%d\n"), indexctr);
        // }

        buttonry_tick();

        if ((blinktick & _BV(0)) != 0) {
            blinktick &= ~_BV(0);

            time_nextsecond();
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
            blinkctr = BLINKCTR_LOAD; 
            sei();
            if (blinkhandler != NULL) {
                blinkhandler(1);
            }

#if 1            
            if (spinned_up != SPIN_START && stall_detect()) {
                spinup_setup();
                spin_disable();
                printf_P(PSTR("STALL\n"));
            }
            
            if (rps == 0 && spinned_up == SPIN_STOP) {
                printf_P(PSTR("RESTART\n"));
                spin_enable();
            }
#else 
            rps = 30;
#endif
        }

    
        if (time_ctr > TICKS_PER_SECOND/2) {
            blinktick |= _BV(1);
        } else {
            blinktick &= ~_BV(1);
        }
        
        //_delay_ms(25);
    }
}

