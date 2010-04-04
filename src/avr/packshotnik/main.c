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

/// Program main
int main() {
    uint8_t i;
    uint8_t byte;
    volatile uint16_t skip = 0;
    uint8_t uart_enabled = 0;

    usart_init(F_CPU/16/19200-1);
    
    printf_P(PSTR("\033[2J\033[HB%s DO COSMONAUTY %02x\n"), BUILDNUM, MCUCSR);

    sei();

    wdt_enable(WDTO_250MS);
    
    set_sleep_mode(SLEEP_MODE_IDLE);
    
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
                        default:
                                    break;
                        }
            
                        printf_P(PSTR("OCR1A=%d ICR1=%d\n"), OCR1A, ICR1);
                        break;
            }
        }
    }
}

