#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <stdio.h>

#include "buttonry.h"
#include "util.h"
#include "timekeep.h"
#include "modes.h"

enum _setstates {
    SET_NONE = 0,
    SET_HOUR,
    SET_MINUTE,
    SET_SEC0,
    SET_YEAR,
    SET_MONTH,
    SET_DAY,
};

/// buttons 1 and 2 states for debouncing
static uint8_t button1_state, button2_state;

static uint8_t set_state;       //<! setup state, see enum _setstates 

#define DDRBUTTONS  DDRD
#define PORTBUTTONS PORTD
#define PINBUTTONS  PIND

#define BUTTON1     7
#define BUTTON2     6


/// Catch button press and release moments, call handler
void debounce(uint8_t port, uint8_t* state, void (*handler)(uint8_t)) {
    if (*state && !port) handler(1);
    if (!*state && port) handler(0);
    *state = port;
}

/// Initialize ports and variables
void buttons_init() {
    DDRBUTTONS &= ~BV2(BUTTON1,BUTTON2);
    PORTBUTTONS |= BV2(BUTTON1,BUTTON2);
    button1_state = button2_state = 0;
}


#define set_blinkhandler(x) { cli();\
                              if (blinkhandler == NULL) {\
                                blinkhandler = x;\
                                skip = 1;\
                              }\
                              sei();\
                            }

/// Handler for button 1: "SET"
void button1_handler(uint8_t on) {
    static uint8_t skip = 0;
    
    if (on) {
        blinkmode_set(blinkmode_get() | 0200);  // don't blink while button is being depressed

        if (skip > 0) {
            skip--;
            return;
        }
        
        switch (set_state) {
            case SET_NONE:
                mode_next();
                skip = 1;
                break;
                
            case SET_HOUR:
                set_blinkhandler(button1_handler);

                time_nexthour();

                fadeto(time_get_hhmm());
                break;
                
            case SET_MINUTE:
                set_blinkhandler(button1_handler);
                
                time_nextminute();

                fadeto(time_get_hhmm());
                break;
                
            case SET_YEAR:
                set_blinkhandler(button1_handler);
                break;     
                       
            case SET_MONTH:
                set_blinkhandler(button1_handler);
                break;
            
            case SET_DAY:
                set_blinkhandler(button1_handler);
                break;
        }
    } else {
        blinkmode_set(blinkmode_get() & 0177);  // re-enable blinking
        cli(); blinkhandler = NULL; skip = 0; sei();
    }
}

/// Handler for button 2, "+"
void button2_handler(uint8_t on) {
    if (on) {
        switch (set_state) {
            case SET_NONE:
                switch (mode_get()) {
                case MMSS:
                    time_set(-1,-1,0);
                    fadeto(time_get_mmss());
                    break;
                case HHMM:
                    set_state = SET_HOUR;
                    blinkmode_set(BLINK_HH);
                    fade_set(FADE_OFF);
                    dotmode_set(DOT_ON);
                    
                    fadeto(time_get_hhmm());
                    break;
                default:
                    break;
                }
                break;
            case SET_HOUR:
                set_state = SET_MINUTE;
                blinkmode_set(BLINK_MM);
                break;
            case SET_MINUTE:
            default:
                set_state = SET_NONE;
                blinkmode_set(BLINK_NONE);
                fade_set(FADE_SLOW);
                dotmode_set(DOT_BLINK);
                break;
        }
    } else {
        cli(); blinkhandler = NULL; sei();
    }
}

uint8_t is_setting() {
    return set_state != SET_NONE;
}

void buttonry_tick() {
    debounce(PINBUTTONS & _BV(BUTTON1), &button1_state, button1_handler);
    debounce(PINBUTTONS & _BV(BUTTON2), &button2_state, button2_handler);
}
