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
    STEP_TEST = 0,
    STEP_LOBO,
    STEP_HUGE,
    STEP_NORM,
    STEP_TINY,
    STEP_NANO,
    STEP_LAST = STEP_NANO,
    STEP_FIRST = STEP_TEST,
} StepMode;

void stepmode_next();
StepMode stepmode_get();
PGM_P stepmode_gettext();

/// Torque
typedef enum _torquemode {
    TORQ_WEAK = 0,
    TORQ_FULL,
    
    TORQ_FIRST = TORQ_WEAK,
    TORQ_LAST = TORQ_FULL,
} TorqueMode;

void torquemode_next();
void torquemode_set(TorqueMode tm);
TorqueMode torquemode_get();
PGM_P torquemode_gettext();

/// Pace
typedef enum _pacemode {
    PACE_1 = 0,
    PACE_2,
    PACE_3,
    PACE_4,
    PACE_5,
    PACE_6,
    
    PACE_FIRST = PACE_1,
    PACE_LAST = PACE_6
} PaceMode;

void pacemode_next();
PaceMode pacemode_get();
PGM_P pacemode_gettext();

#endif
