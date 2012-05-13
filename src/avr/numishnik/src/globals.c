#include <inttypes.h>

#include "rtc.h"

volatile uint16_t blinkcounter;
volatile uint8_t blinkdot;

volatile RTC_TIME the_time;

