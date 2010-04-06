#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "util.h"
#include "lobo.h"
#include "modes.h"
#include "hcms3966.h"
#include "buttonry.h"
#include "ircontrol.h"

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

#define FULL_CIRCLE     610U
#define SCALE           100

typedef enum _psstate {
    PS_BEGIN1,
    PS_BEGIN2,
    PS_BEGIN3,
    PS_STEP,
    PS_PACE,
    PS_END
} PSState;




PSState  ps_state = PS_END;
uint16_t ps_ctr = 0;
uint16_t ps_pace = 15;

volatile uint16_t framecount = 0;

volatile uint16_t   lobo_thresh;
volatile uint16_t   lobo_pulse;

typedef enum {
    LOBO_STOP,
    LOBO_RUN,
    LOBO_PAUSE,
} LoboState;

volatile LoboState    lobo_state = LOBO_STOP;

ISR(INT0_vect) {
    lobo_pulse += SCALE;
    if (lobo_pulse > lobo_thresh) {
        lobo_pulse -= lobo_thresh;
        lobo_state = LOBO_PAUSE;
    }    
}

void lobo_step() {
    if (lobo_state == LOBO_RUN) {
        lobo_run(1);
        _delay_us(60);
    }
    lobo_run(0);
    switch (torquemode_get()) {
        case TORQ_PUNY:
            _delay_us(100);
            break;
        case TORQ_FULL:
            _delay_us(60);
            break;
    }
}


void packshot_start() {
    ps_state = PS_BEGIN1;
    ps_ctr = 20;
    
    switch (stepmode_get()) {
        case STEP_TINY: framecount = 120; break;
        case STEP_NORM: framecount = 60;  break;
        case STEP_HUGE: framecount = 30;  break;
        case STEP_LOBO: framecount = 10;   break;
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

void packshot_do() {
    if (ps_ctr > 0) --ps_ctr;
    
    if (ps_ctr == 0) {
        switch (ps_state) {
            case PS_BEGIN1:
                display_ps(PSTR("\001  3"));
                ps_ctr = 15;
                ps_state = PS_BEGIN2;
                break;
            case PS_BEGIN2:        
                display_ps(PSTR("\001  2"));
                ps_ctr = 15;
                ps_state = PS_BEGIN3;
                break;
            case PS_BEGIN3:
                display_ps(PSTR("\001  1"));
                ps_ctr = 15;
                ps_state = PS_STEP;
                break;
            case PS_STEP:
                hcms_loadcw(0x44);
                display_u(framecount);
                if (framecount > 0) {
                    --framecount;
                    lobo_state = LOBO_RUN;
                    ps_state = PS_PACE;
                } else {
                    ps_state = PS_END;
                    display_ps(PSTR("DONE"));
                    buttons_init();
                }
                break;
            case PS_PACE:
                if (lobo_state == LOBO_PAUSE) {
                    // blank the display and release the shutter
                    hcms_loadcw(0x40);
                    ps_ctr = ps_pace;
                    ps_state = PS_STEP;
                    irc_shutter();
                }
                break;
            case PS_END:
                break;
        }
    }
}
