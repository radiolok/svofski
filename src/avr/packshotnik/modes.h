#ifndef _MODES_H_
#define _MODES_H_

#include <avr/pgmspace.h>

/// Blinking modes
enum _blinkmode {
    BLINK_NONE = 0,
    BLINK_ALL = 3,
    BLINK_SUPPRESS = 0200,  //!< To be OR'ed with current mode
};

void blinkmode_set(uint8_t mode);

uint8_t blinkmode_get();

/// Step modes
typedef enum _stepmode {
    STEP_TINY = 0,
    STEP_NORM,
    STEP_HUGE,
    STEP_LOBO,
    STEP_LAST = STEP_LOBO
} StepMode;

void stepmode_next();
StepMode stepmode_get();
PGM_P stepmode_gettext();

/// Torque
typedef enum _torquemode {
    TORQ_PUNY = 0,
    TORQ_FULL,
    
    TORQ_FIRST = TORQ_PUNY,
    TORQ_LAST = TORQ_FULL,
} TorqueMode;

void torquemode_next();
TorqueMode torquemode_get();
PGM_P torquemode_gettext();

/// Pace
typedef enum _pacemode {
    PACE_SLOW = 0,
    PACE_NORM = 1,
    PACE_FAST = 2,
    
    PACE_FIRST = PACE_SLOW,
    PACE_LAST = PACE_FAST
} PaceMode;

void pacemode_next();
PaceMode pacemode_get();
PGM_P pacemode_gettext();

#endif
