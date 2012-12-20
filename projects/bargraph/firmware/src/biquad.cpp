#if 0
#include <stdint.h>
#include <stdlib.h>
#include "biquad.h"

const size_t SAMPLERATE = 28000;

Filter filters[] = {
    Biquad<65536>(SAMPLERATE, 44.0, 2.0)
};
Filter f1 = Biquad<65536>(SAMPLERATE, 44.0, 2.0);
Filter f2 =   Biquad<65536>(SAMPLERATE, 60.0, 1.5);
Filter f3 =     Biquad<32768>(SAMPLERATE, 80.0, 1.5);
Filter f4 =       Biquad<32768>(SAMPLERATE, 107.0, 1.1);
Filter f5 =        Biquad<32768>(SAMPLERATE, 145.0, 1.0); 
Filter f6 =        Biquad<32768>(SAMPLERATE, 195.0, 0.8); 
Filter f7 =        Biquad<32768>(SAMPLERATE, 264.0, 0.75);
Filter f8 =         Biquad<32768>(SAMPLERATE, 356.0, 0.74);
Filter f9 =         Biquad<32768>(SAMPLERATE, 480.0, 1.0);
Filter f10 =         Biquad<32768>(SAMPLERATE, 647.0, 1.0);
Filter f11 =         Biquad<32768>(SAMPLERATE, 872.0, 1.0);
Filter f12 =         Biquad<32768>(SAMPLERATE, 1175.0, 1.0);
Filter f13 =         Biquad<32768>(SAMPLERATE, 1584.0, 1.0);
Filter f14 =         Biquad<32768>(SAMPLERATE, 2135.0, 1.0);
Filter f15 =         Biquad<32768>(SAMPLERATE, 2878.0, 1.0);
Filter f16 =         Biquad<32768>(SAMPLERATE, 3880.0, 1.0);
Filter f17 =         Biquad<32768>(SAMPLERATE, 5230.0, 1.0);
Filter f18 =         Biquad<32768>(SAMPLERATE, 7050.0, 1.0);
Filter f19 =         Biquad<32768>(SAMPLERATE, 9504.0, 1.0);
Filter f20 =         Biquad<32768>(SAMPLERATE, 12811.0, 0.6);

#endif
