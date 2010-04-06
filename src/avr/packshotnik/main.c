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

ISR(INT0_vect) {
    lobo_index();
}

ISR(TIMER0_OVF_vect) {
    if (blinkctr > 0) blinkctr--;
    blinkbits |= _BV(7);
}

void timers_init() {
    TCCR0 = 5;  // clk/1024
    TIMSK |= _BV(TOIE0);
}

uint8_t run_lobo_run(uint8_t flag) {
    static int8_t count;
    static uint8_t state = 10;
    
    if (flag == 255) {
        return state == 10;
    } else if (flag == 254) {
        state = 10;
    } else if (flag == 1) {
        lobo_reset_pulsecount();
        count = 100;
        state = 0;
    }

    if (state == 10) {
        return 0;
    }
    
    if (count > 0) count--;
    
    lobo_run(1);
    
    switch (state) {
        case 0:
            _delay_us(40);
            if (count == 0) {
                count = 100;
                state = 1;
            }
            break;
        case 1:
            _delay_us(60);
            if (count == 0) {
                count = 100;
                state = 2;
            }
            break;
        case 2:
            _delay_us(80);
            break;
        case 3:
            _delay_us(60);
            if (count == 0) {
                state = 4;
                count = -1;
            }
            break;
        case 4:
            _delay_us(40);
            break;
    }
    
    switch (state) {
        case 0:
        case 1:
        case 2:
            if (flag == 2) {
                count = 10;
                state = 3;
            }
            break;
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
    
    return 0;
}

typedef enum _psstate {
    PS_BEGIN1,
    PS_BEGIN2,
    PS_BEGIN3,
    PS_STEP,
    PS_PACE,
    PS_END
} PSState;


uint8_t ps_state = PS_END;
uint8_t ps_ctr = 0;
uint8_t ps_pace = 15;

volatile uint8_t brakethresh = 0;
volatile uint8_t steppulses = 0;
uint16_t revsteps = 0;


void lobo_step() {
    if (lobo_get_pulsecount() < steppulses) {
        run_lobo_run((lobo_get_pulsecount() >= brakethresh) ? 2 : 0);
    } else {
        run_lobo_run(254);
    }
}


void packshot_start() {
    ps_state = PS_BEGIN1;
    ps_ctr = 20;
    
    switch (stepmode_get()) {
        case STEP_TINY: steppulses = 2;  brakethresh = 1;   revsteps = 305;    break;
        case STEP_NORM: steppulses = 10; brakethresh = 8;   revsteps = 61;    break;
        case STEP_HUGE: steppulses = 18; brakethresh = 16;  revsteps = 34;     break;
        case STEP_LOBO: steppulses = 38; brakethresh = 34;  revsteps = 16;     break;
    }
    
    switch (pacemode_get()) {
        case PACE_SLOW: ps_pace = 64;   break;
        case PACE_NORM: ps_pace = 32;   break;
        case PACE_FAST: ps_pace = 16;    break;
    }
}

void packshot_do() {
    if (ps_ctr > 0) --ps_ctr;
    
    if (ps_ctr == 0) {
        switch (ps_state) {
            case PS_BEGIN1:
                display_ps(PSTR("GO 3"));
                ps_ctr = 15;
                ps_state = PS_BEGIN2;
                break;
            case PS_BEGIN2:        
                display_ps(PSTR("GO 2"));
                ps_ctr = 15;
                ps_state = PS_BEGIN3;
                break;
            case PS_BEGIN3:
                display_ps(PSTR("GO 1"));
                ps_ctr = 15;
                ps_state = PS_STEP;
                break;
            case PS_STEP:
                display_ps(PSTR("----"));
                if (revsteps > 0) {
                    --revsteps;
                    run_lobo_run(1);
                    ps_state = PS_PACE;
                } else {
                    ps_state = PS_END;
                    display_ps(PSTR("DONE"));
                    buttons_init();
                }
                break;
            case PS_PACE:
                if (run_lobo_run(255)) {
                    printf_P(PSTR("RS%03d PC%03d\n"), revsteps, lobo_get_pulsecount());
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
    
    display_ps(PSTR("WOPR"));
    _delay_ms(250);

    buttons_init();
    
    timers_init();

    wdt_enable(WDTO_250MS);
    
    run_lobo_run(254);
    
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
                            if (run_lobo_run(255)) {
                                run_lobo_run(1);
                            }
                            
                            break;
                        default:
                            break;
                        }
                        printf_P(PSTR("PulseCount=%d, PiB=%02x PiD=%02x PB=%02x PC=%02x PD=%02x steps=%d\n"), lobo_get_pulsecount(), PINB, PIND, PORTB, PORTC, PORTD, steps);
                        break;
            }
        }
        
        if (!run_lobo_run(255)) {
            lobo_step();
        }
        
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

