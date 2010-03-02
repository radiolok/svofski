#ifndef _TIMEKEEP_H_
#define _TIMEKEEP_H_

#include <inttypes.h>

void time_set(int8_t hh, int8_t mm, int8_t ss);
uint16_t time_get_hhmm();
uint16_t time_get_mmss();
void time_nexthour();
void time_nextminute();
void time_nextsecond();    

#endif
