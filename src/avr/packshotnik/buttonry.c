#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <stdio.h>

#include "buttonry.h"
#include "util.h"
#include "common.h"
#include "modes.h"
#include "hcms3966.h"
#include "lobo.h"

enum _setstates {
    SET_NONE = 0,
    SET_STEP,
    SET_TORQ,
    SET_PACE,
    SET_GO,
};

/// buttons 1 and 2 states for debouncing
static uint8_t button1_state, button2_state;

static uint8_t set_state;       //<! setup state, see enum _setstates 

static uint8_t blink;

static PGM_P currentModeText;

#define DDRBU1       DDRD
#define PORTBU1      PORTD
#define PINBU1       PIND
#define BUTTON1      3

#define DDRBU2       DDRB
#define PORTBU2      PORTB
#define PINBU2       PINB
#define BUTTON2      7

static const prog_char P_FRMS[] = "FRMS";
static const prog_char P_TORQ[] = "TORQ";
static const prog_char P_PACE[] = "PACE";
static const prog_char P_GO_[]  = "GO  ";
static const prog_char P__GO[]  = "  GO";
static const prog_char P_OK[]   = " OK ";



void blinker(uint8_t);

/// Catch button press and release moments, call handler
void debounce(uint8_t port, uint8_t* state, void (*handler)(uint8_t)) {
    if (*state && !port) handler(1);
    if (!*state && port) handler(0);
    *state = port;
}

#define set_blinkhandler(x) { cli();\
                              blinkhandler = x;\
                              sei();\
                            }


static void swState(uint8_t state, PGM_P text) {
    currentModeText = text;
    set_state = state;
}


/// Initialize ports and variables
void buttons_init() {
    DDRBU1 &= ~_BV(BUTTON1);
    PORTBU1 |= _BV(BUTTON1);
    
    DDRBU2 &= ~_BV(BUTTON2);
    PORTBU2 |= _BV(BUTTON2);
    button1_state = button2_state = 0;
    
    swState(SET_NONE, P_OK);
    set_blinkhandler(blinker);
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
        
        blink = 0;
        
        switch (set_state) {
            case SET_NONE:
                swState(SET_STEP, P_FRMS);
                break;
                
            case SET_STEP:
                swState(SET_TORQ, P_TORQ);
                break;
                
            case SET_TORQ:
                swState(SET_PACE, P_PACE);
                break;
                
            case SET_PACE:
                swState(SET_GO, P__GO);
                break;     
                       
            case SET_GO:
                swState(SET_NONE, P_OK);
                break;
        }
        blink_haste();
    } 
}

void blinker(uint8_t on) {
    if (!blink) {
        display_ps(currentModeText);
    }
    
    switch (set_state) {
        case SET_NONE:
            break;
        case SET_STEP:
            if (blink) {
                display_ps(stepmode_gettext());
            } 
            break;
        case SET_TORQ:
            if (blink) {
                display_ps(torquemode_gettext());
            }
            break;
        case SET_PACE:
            if (blink) {
                display_ps(pacemode_gettext());
            }
            break;
        case SET_GO:
            if (blink) {
                display_ps(P_GO_);
            }
            break;
        default:
            break;
    }
    
    blink = !blink;
}

/// Handler for button 2, "+"
void button2_handler(uint8_t on) {
    if (on) {
        switch (set_state) {
            case SET_NONE:
                swState(SET_GO, P__GO);
                break;
            case SET_STEP:
                stepmode_next();
                break;
            case SET_TORQ:
                torquemode_next();
                break;
            case SET_PACE:
                pacemode_next();
                break;
            case SET_GO:
                set_blinkhandler(NULL);
                if (packshot_isactive()) {
                    packshot_stop();
                } else {
                    packshot_start();
                }
                break;
        }
        blink = 1;
        blink_haste();
    }
}

uint8_t is_setting() {
    return set_state != SET_NONE;
}

void buttonry_tick() {
    //printf_P(PSTR("%02x %02x "), PINBU1, PINBU2);
    debounce(PINBU1 & _BV(BUTTON1), &button1_state, button1_handler);
    debounce(PINBU2 & _BV(BUTTON2), &button2_state, button2_handler);
}
