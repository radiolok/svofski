#ifndef HCMS_
#define HCMS_

#include "util.h"

#define P_BLANK PORTB
#define D_BLANK DDRB
#define B_BLANK 1

#define P_HCMS PORTC
#define D_HCMS DDRC

#define DCLK    0
#define DRS     1
#define DDIN    2
#define DCE_N   3  

#define X_HCMS BV4(DCLK,DRS,DDIN,DCE_N)


void hcms_init();

void hcms_loadcw(uint8_t w);

void hcms_boo();

#endif