#include <inttypes.h>

#include "msp430fr5739.h"
#include "FraunchLeds.h"

#define PJBITS (BIT0+BIT1+BIT2+BIT3)
#define P3BITS (BIT6+BIT7+BIT5+BIT4)

FraunchLeds blueleds;

FraunchLeds::FraunchLeds()
{
	// Enable LEDs
	P3OUT &= ~P3BITS;
	P3DIR |= P3BITS;
	PJOUT &= ~PJBITS;
	PJDIR |= PJBITS;
}

uint8_t FraunchLeds::Set(int n, int state)
{
	uint8_t bit = 1 << n;
	uint8_t current = Get();
	uint8_t newval = current;
	
	switch (state) {
	case 0:
		newval = current & ~bit;
		break;
	case 1:
		newval = current | bit;
		break;
	case -1:
		newval = current ^ bit;
		break;
	default:
		break;
	}
	
	Set(newval);
	
	return newval;
}

uint8_t FraunchLeds::Set(uint8_t bits)
{
	PJOUT = (PJOUT & ~PJBITS) | (bits & PJBITS);
	P3OUT = (P3OUT & ~P3BITS) | (bits & P3BITS);

	return bits;
}

uint8_t FraunchLeds::Get() 
{
	return (PJOUT & 017) | (P3OUT & 0360);
}
