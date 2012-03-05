#include <inttypes.h>
#include "util.h"

uint8_t frombcd(uint8_t x) {
    return _frombcd(x);
}

uint8_t bcd_increment(uint8_t x) {
	x++;
	if ((x & 0x0f) >= 0x0a)
		x += 0x10 - 0x0a;
	if (x >= 0xa0) 
	   x = 0;
	return x;
}

uint16_t tobcd16(uint16_t x) {
    uint16_t y;
    
    y  = x % 10;        x /= 10;       
    y |= (x % 10)<<4;   x /= 10;
    y |= (x % 100)<<8;  x /= 10; 
    y |= 0xf000;
    
    return y;
}


