#ifndef HCMS_
#define HCMS_

#include <avr/pgmspace.h>
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
void hcms_quad(PGM_P msg, char *rmsg);
void hcms_boo();

void display_ps(PGM_P msg);
void display_s(char* msg);
void display_u(uint16_t u);

#endif