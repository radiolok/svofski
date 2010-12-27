// luts.h: 
// header file for LUT interface
//
// (Created: 2004-03-24, Modified: 2004-03-24, Cearn)

#ifndef GBA_
#define GBA_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// lut includes (declaration only)
#include "sinlut.h"
#include "divlut.h"

// --- macros that give the sine and cosine values ---
// NOTICE ON USE:
// yes, lut_sin and lut_cos are macros, not LUTs. I'm doing things this
// way because it's more portable. As long as the sine lut is 
// 2^n in size you can use these macros. If you want bigger LUTs, 
// simply use excellut again with a modified size (and copy&paste 
// this section in.
// If you wan't to be pig-headed and use either non power-of-two 
// sizes or separate LUTs for sin and cos, simply alter the 
// code, and you're done (well, recompile and you're done. 
//
// These macros are used by the affine routines in affine.c
// If you change this, you'll have to recompile it or rotation 
// will be screwed.
//

#define SIN_MASK (SIN_SIZE-1)
#define lut_sin(x)	_sinLUT[x]
#define lut_cos(x)	_sinLUT[(x+(SIN_SIZE>>2)) & SIN_MASK]

#ifdef __cplusplus
}
#endif

#endif // GBA_

