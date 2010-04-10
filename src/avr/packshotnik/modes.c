#include <inttypes.h>
#include "util.h"
#include "modes.h"

////
//// Blink mode
////
static volatile uint8_t blinkmode;     //!< current blinking mode

void blinkmode_set(uint8_t mode) {
    blinkmode = mode;
}

inline uint8_t blinkmode_get() { return blinkmode; }

/// -- stepmodes
static volatile uint8_t stepmode = STEP_TEST;

void stepmode_next() {
    stepmode ++;
    if (stepmode > STEP_LAST) {
        stepmode = STEP_FIRST;
    }
}

inline StepMode stepmode_get() {
    return stepmode;
}

PGM_P stepmode_gettext() {
    switch (stepmode) {
    case STEP_NANO: return PSTR(" 240");
    case STEP_TINY: return PSTR(" 120");
    case STEP_NORM: return PSTR("  60");
    case STEP_HUGE: return PSTR("  30");
    case STEP_LOBO: return PSTR("  10");
    case STEP_TEST: return PSTR("test");
    }
    return 0;
}

/// -- torque modes
static volatile TorqueMode torquemode = TORQ_WEAK;

void torquemode_next() {
    torquemode++;
    if (torquemode > TORQ_LAST) {
        torquemode = TORQ_FIRST;
    }
}

inline TorqueMode torquemode_get() {
    return torquemode;
}

PGM_P torquemode_gettext() {
    switch (torquemode) {
        case TORQ_WEAK: return PSTR("weak");
        case TORQ_FULL: return PSTR("good");
    }
    return 0;
}

/// -- pace modes
static volatile PaceMode pacemode = PACE_3;

void pacemode_next() {
    pacemode++;
    if (pacemode > PACE_LAST) {
        pacemode = PACE_FIRST;
    }
}

inline PaceMode pacemode_get() {
    return pacemode;
}

PGM_P pacemode_gettext() {
    switch (pacemode) {
        case PACE_1: return PSTR("0.5s");
        case PACE_2: return PSTR("  1s");
        case PACE_3: return PSTR("  3s");
        case PACE_4: return PSTR("  5s");
        case PACE_5: return PSTR(" 10s");
        case PACE_6: return PSTR(" 20s");
    }
    return 0;
}