#ifndef _LOBO
#define _LOBO

#include <inttypes.h>

#define D_LOBO  DDRD
#define P_LOBO  PORTD
#define BMOTOR  6

void lobo_init();

void lobo_run(uint8_t run);

uint8_t lobo_is_running();

uint8_t lobo_get_pulsecount();

void lobo_reset_pulsecount();

void lobo_step();

void packshot_start();

void packshot_do();

#endif
