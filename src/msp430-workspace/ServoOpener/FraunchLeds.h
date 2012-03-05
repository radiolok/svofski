#ifndef FRAUNCHLEDS_H_
#define FRAUNCHLEDS_H_

#include <inttypes.h>

class FraunchLeds
{
public:
	FraunchLeds();
	uint8_t Set(int n, int state);
	uint8_t Set(uint8_t bits);
	uint8_t Get(); 
};

extern FraunchLeds blueleds;

#endif /*FRAUNCHLEDS_H_*/
