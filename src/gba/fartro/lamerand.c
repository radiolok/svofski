//#include <stdlib.h>
#include "lamerand.h"

static u16 lamerand_slut[257];

static int randidx = 42;

u32 _math_rand_seed = 0x600D5EED;

u16 math_rand (u16 max) {
	u32 rnd;
	u32 div;

	if (max > 0x7FFF) {
		max = 0x7FFF;
	}

	div = 0x8000 / (max + 1);

	_math_rand_seed = _math_rand_seed * 1103515245 + 12345;
	rnd = (_math_rand_seed >> 16) & 0x7FFF;

	return rnd / div;
}

void lame_randomize() {
	int i;

	for (i = 0; i < 257; i++) {
        lamerand_slut[i] = math_rand(LAMERAND_MAX);
	}
}

u16 lamerand() {
	u16 result = lamerand_slut[randidx];
	randidx += result;
	randidx = randidx &0xff;
	if (randidx < 256) {
		randidx++;
	}
	return result;
}
