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
static volatile uint8_t stepmode;

void stepmode_next() {
    stepmode ++;
    if (stepmode > STEP_LAST) {
        stepmode = STEP_TINY;
    }
}

inline StepMode stepmode_get() {
    return stepmode;
}

PGM_P stepmode_gettext() {
    switch (stepmode) {
    case STEP_TINY: return PSTR("tiny");
    case STEP_NORM: return PSTR("norm");
    case STEP_HUGE: return PSTR("huge");
    case STEP_LOBO: return PSTR("lobo");
    }
    return 0;
}

/// -- torque modes
static volatile TorqueMode torquemode;

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
        case TORQ_PUNY: return PSTR("puny");
        case TORQ_FULL: return PSTR("full");
    }
    return 0;
}

/// -- pace modes
static volatile PaceMode pacemode = PACE_NORM;

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
        case PACE_SLOW: return PSTR("slow");
        case PACE_NORM: return PSTR("norm");
        case PACE_FAST: return PSTR("fast");
    }
    return 0;
}