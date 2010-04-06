///\file main.c
///\author Viacheslav Slavinsky
///
///\brief Packshotnik
///
/// \mainpage A rotating platform with automated shutter release for cameras
/// \section Files
/// - main.c    main file
/// - util.c    Calendar and other utils
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

#include "usrat.h"
#include "hcms3966.h"
#include "lobo.h"
#include "common.h"
#include "buttonry.h"
#include "modes.h"

#define BLINKCTR_LOAD   20

uint8_t blinkctr = 0;
uint8_t blinkbits = 0;

void blink_haste() {
    cli(); blinkctr = 0; sei();
}


ISR(TIMER0_OVF_vect) {
    if (blinkctr > 0) blinkctr--;
    blinkbits |= _BV(7);
}

void timers_init() {
    TCCR0 = 5;  // clk/1024
    TIMSK |= _BV(TOIE0);
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
    lobo_index();
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
                    hcms_loadcw(0x40);
                    printf_P(PSTR("FC%03d PC%03d\n"), framecount, lobo_pulse);
                    ps_ctr = ps_pace;
                    ps_state = PS_STEP;
                }
                break;
            case PS_END:
                break;
        }
    }
}

/// Program main
int main() {
    uint8_t i;
    uint8_t byte;
    volatile uint16_t steps = 0;
    uint8_t uart_enabled = 0;

    usart_init(F_CPU/16/19200-1);
    
    printf_P(PSTR("\033[2J\033[HB%s DO COSMONAUTY %02x\n"), BUILDNUM, MCUCSR);

    sei();

    
    set_sleep_mode(SLEEP_MODE_IDLE);

    lobo_init();
    
    hcms_init();
    hcms_boo();
    _delay_ms(50);
    display_ps(PSTR("WOPR"));
    _delay_ms(250);

    buttons_init();
    
    timers_init();

    wdt_enable(WDTO_250MS);
    
    //run_lobo_run(254);
    
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
                        case 'm':
                            steps = 0;
                            //if (run_lobo_run(255)) {
                            //   run_lobo_run(1);
                            //}
                            
                            break;
                        default:
                            break;
                        }
                        printf_P(PSTR("PulseCount=%d, PiB=%02x PiD=%02x PB=%02x PC=%02x PD=%02x steps=%d\n"), lobo_get_pulsecount(), PINB, PIND, PORTB, PORTC, PORTD, steps);
                        break;
            }
        }
        
        //if (!run_lobo_run(255)) {
        //    lobo_step();
        //}
        
        lobo_step();
        
        if (blinkbits & _BV(7)) {
            blinkbits &= ~_BV(7);
            buttonry_tick();
            packshot_do();
        }
        
        if (blinkctr == 0) {
            cli(); blinkctr = BLINKCTR_LOAD; sei();
            if (blinkhandler != NULL) {
                blinkhandler(1);
            }
        }
    }
}

