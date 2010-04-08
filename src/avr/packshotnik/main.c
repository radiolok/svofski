///\file main.c
///\author Viacheslav Slavinsky
///
///\brief Packshotnik
///
/// \mainpage A rotating platform with automated shutter release for cameras
/// \section Files
/// - main.c    main file
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
#include "ircontrol.h"

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
    TCCR0 = 5;              // clk/1024
    TIMSK |= _BV(TOIE0);
}

void carrier(uint8_t);
extern uint16_t off_count;

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
    
    irc_init();

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
                            carrier(1);
                            break;
                        case '1':
                            OCR1A--;
                            break;
                        case '2':
                            OCR1A++;
                            break;
                        case 'q':
                            off_count--;
                            break;
                        case 'w':
                            off_count++;
                            break;
                        case 's':
                            irc_shutter();
                            break;
                        default:
                            break;
                        }
                        printf_P(PSTR("PiB=%02x PiD=%02x PB=%02x PC=%02x PD=%02x OCR1A=%d off=%d\n"), PINB, PIND, PORTB, PORTC, PORTD, OCR1A, off_count);
                        break;
            }
        }
        
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

