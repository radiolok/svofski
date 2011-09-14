// color.c:
// color and palette routines
//
// (Created: 2003-05-05, Modified: 2004-01-06, Cearn)

#include "types.h"
#include "regs.h"
#include "core.h"
#include "color.h"

// load up a default gradient palette
void pal_load_def()
{
	int ii;
	const int SHADES=16, PALSIZE=256;
	COLOR color=0x0000, dcolor= _RGB15(1, 1, 1);
	COLOR *pal= pal_bg_mem;
	u32 offset=0;

	// load gradient palette. MAJOR optimisations possible
	// This could be done sooo well in assembler...
	for(ii=0; ii<SHADES; ii++)
	{
		offset=0;//SHADES<<1;
		*(pal+offset)= color & REDMASK;			offset += SHADES;
		*(pal+offset)= color & GREENMASK;		offset += SHADES;
		*(pal+offset)= color & BLUEMASK;		offset += SHADES;

		*(pal+offset)= color & ~REDMASK;		offset += SHADES;
		*(pal+offset)= color & ~GREENMASK;		offset += SHADES;
		*(pal+offset)= color & ~BLUEMASK;		offset += SHADES;

		// add grays last
		*(pal+ii+0xe0)= color;
		color += dcolor;
		*( pal+ii+0xe1)= color;
		// update stuff
		pal++;
		color += dcolor;
	}
	// set the rest to 0
	memset16(pal_bg_mem+0x60, 0, PALSIZE>>2);
}
