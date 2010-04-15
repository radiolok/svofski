#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "util.h"
#include "lobo.h"
#include "modes.h"
#include "hcms3966.h"
#include "buttonry.h"
#include "ircontrol.h"

#define FULL_CIRCLE     518
#define SCALE           125 //65536/FULL_CIRCLE

#define LWD 600

typedef enum _psstate {
    PS_BEGIN1,
    PS_BEGIN2,
    PS_BEGIN3,
    PS_TURN,
    PS_PACE,
    PS_PACE2,
    PS_END
} PSState;

typedef enum {
    LOBO_STOP,
    LOBO_RUN,
    LOBO_PAUSE,
} LoboState;


PSState  ps_state = PS_END;
uint16_t ps_ctr = 0;
uint16_t ps_pace = 15;

volatile uint16_t framecount = 0;

volatile uint16_t   lobo_thresh;
volatile uint16_t   lobo_pulse;
volatile uint16_t    lobo_watchdog;

volatile LoboState    lobo_state = LOBO_STOP;

/// Init Lobo
void lobo_init() {
    DDRD &= _BV(2);     // PORTD.2 is input, INT0
    
    MCUCR |= _BV(ISC00);  // any logical change generates request
    GICR |= _BV(INT0);
    
    D_LOBO |= _BV(BMOTOR);
    lobo_ctrl(0);
}

/// Run or stop the motor
void lobo_ctrl(uint8_t on) {
    if (on) {
        P_LOBO |= _BV(BMOTOR);
    } else {
        P_LOBO &= ~_BV(BMOTOR);
    } 
}

/// Set motor turning state: RUN/PAUSE/STOP. 
void lobo_setstate(LoboState s) {
    cli();
    lobo_state = s;
    sei();
    if (s == LOBO_RUN) {
        lobo_watchdog = LWD;
    }
}


/// Optical encoder interrupt
ISR(INT0_vect) {
    lobo_pulse += SCALE;
    lobo_watchdog = LWD;
    if (lobo_pulse >= lobo_thresh) {
        lobo_pulse -= lobo_thresh;
        lobo_setstate(LOBO_PAUSE);
    }    
}

/// Loop routine: a primitive PWM driver. Called from the main loop continuously.
/// On-cycle is always 60us, off-cycle is variable
uint8_t lobo_step() {
    if (lobo_state == LOBO_RUN) {
        lobo_ctrl(1);
        _delay_us(60);
        if (--lobo_watchdog == 0) {
            // motor appears to be stuck
            lobo_ctrl(0);
            lobo_setstate(LOBO_STOP);
            return 0;
        }
    }
    lobo_ctrl(0);
    switch (torquemode_get()) {
        case TORQ_WEAK:
             // longer off-cycles are possible but this seems to be the reasonable limit
            _delay_us(140);
            break;
        case TORQ_FULL:
            _delay_us(60);
            break;
    }
    
    return 1;
}

/// Begin the shooting
void packshot_start() {
    ps_state = PS_BEGIN1;
    ps_ctr = 20;
}

void p(uint8_t x) {
    printf_P(PSTR("%d: thresh=%d pulse=%d\n"), x, lobo_thresh, lobo_pulse);
}

static void packshot_calc() {
    switch (stepmode_get()) {
        case STEP_NANO: framecount = 259; break;
        case STEP_TINY: framecount = 120; break;
        case STEP_NORM: framecount = 60;  break;
        case STEP_HUGE: framecount = 30;  break;
        case STEP_LOBO: framecount = 10;  break;
        case STEP_TEST: framecount = 0;   break;
    }
    
    switch (pacemode_get()) {
        case PACE_1: ps_pace = 15;   break;
        case PACE_2: ps_pace = 30;   break;
        case PACE_3: ps_pace = 90;   break;
        case PACE_4: ps_pace = 150;  break;
        case PACE_5: ps_pace = 300;  break;
        case PACE_6: ps_pace = 600;  break;
    }
    
    lobo_thresh = FULL_CIRCLE*SCALE/framecount;
    lobo_pulse = 0;
}

/// true if shooting is in progress
inline uint8_t packshot_isactive() {
    return ps_state != PS_END;
}


/// Peacefully terminate shooting: assume current frame to be the final frame
void packshot_stop() {
    framecount = 0;
}

/// Fail with drama
void packshot_fail() {
    display_ps(PSTR("FAIL"));
    framecount = 0;
    lobo_setstate(LOBO_STOP);
    ps_state = PS_END;
    buttons_init();
}

/// Higher-level loop routine. Called on blinkbit (some 30 times/sec).
void packshot_do() {
    if (ps_ctr > 0) --ps_ctr;
    
    if (ps_ctr == 0) {
        switch (ps_state) {
            case PS_BEGIN1:
                display_ps(PSTR("\0013  "));
                ps_ctr = 15;
                ps_state = PS_BEGIN2;
                break;
            case PS_BEGIN2:        
                display_ps(PSTR("\001 2 "));
                ps_ctr = 15;
                ps_state = PS_BEGIN3;
                break;
            case PS_BEGIN3:
                display_ps(PSTR("\001  1"));
                ps_ctr = 15;
                lobo_setstate(LOBO_PAUSE);

                packshot_calc();
                p(0);
                ps_state = PS_PACE;
                break;
            case PS_TURN:
                // full brightness display while turning
                hcms_loadcw(0x44);
                display_u(framecount);
                
                if (framecount > 0) {
                    --framecount;
                    lobo_setstate(LOBO_RUN);
                    ps_state = PS_PACE;
                } else {
                    ps_state = PS_END;
                    display_ps(PSTR("DONE"));
                    buttons_init();
                }
                break;
            case PS_PACE:
                if (lobo_state == LOBO_PAUSE) {
                    p(1);
                    // wait until oscillations stop
                    ps_ctr = 15;
                    ps_state = PS_PACE2;
                }
                break;
            case PS_PACE2:
                // subdue the display and release the shutter
                p(2);
                hcms_loadcw(0x41);
                ps_ctr = ps_pace;
                ps_state = PS_TURN;
                irc_shutter();
                break;
            case PS_END:
                break;
        }
    }
}
