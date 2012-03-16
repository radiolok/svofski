#pragma once

#include "rtc.h"

extern volatile uint16_t blinkcounter;
extern volatile uint8_t blinkdot;

extern volatile RTC_TIME the_time;

void spi_wait();
