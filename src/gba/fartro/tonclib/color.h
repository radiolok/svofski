// color.h: 
// simple color defines and utilities
// palette stuff
//
// (Created: 2003-05-05, Modified: 2004-01-06, Cearn)

#ifndef GBA_COLOR
#define GBA_COLOR

#include "types.h"

// === CONSTANTS ======================================================

// --- color stuff ---
// gba uses 5.5.5 format, so...
#define COLORMASK     0x1f
#define REDMASK     0x001f
#define GREENMASK   0x03e0
#define BLUEMASK    0x7c00
#define REDSHIFT         0
#define GREENSHIFT       5
#define BLUESHIFT       10

#define CLR_BLACK	0x0000
#define CLR_RED		0x001f
#define CLR_GREEN	0x03e0
#define CLR_YELLOW	0x03ff
#define CLR_BLUE	0x7c00
#define CLR_MAG		0x7c1f
#define CLR_CYAN	0x7fe0
#define CLR_WHITE	0x7fff

// --- palette stuff ---

#define PAL_MAX 256
// default colors:

#define DEF_WHITE  0xff
#define DEF_RED    0x0f
#define DEF_GREEN  0x1f
#define DEF_BLUE   0x2f
#define DEF_CYAN   0x3f
#define DEF_MAG    0x4f
#define DEF_YELLOW 0x5f
#define DEF_BLACK  0xe0
#define DEF_GRAY   0xf0

#define pal_bg_mem    ((COLOR*)MEM_PAL)
#define pal_obj_mem   ((COLOR*)(MEM_PAL+0x0200))


// === PROTOTYPES =====================================================

// --- palette stuff ---
void pal_load_def();

// === MACROS =========================================================

// background palette
#define pal_bg_set(color, entry)					\
	(pal_bg_mem[entry]= color)
#define pal_bg_mset(color, first, last)				\
	memset16(pal_bg_mem+(first), color, (last)-(first)+1)
#define pal_bg_copy(colors, first, last)			\
	memcpy16(pal_bg_mem+(first), colors, (last)-(first)+1)
#define pal_bg_rotate(first, last)					\
	memrot16(pal_bg_mem+(first), colors, (last)-(first)+1)

// obj palette
#define pal_obj_set(color, entry)					\
	(pal_obj_mem[entry]= color)
#define pal_obj_mset(color, first, last)			\
	memset16(pal_obj_mem+(first), color, (last)-(first)+1)
#define pal_obj_copy(colors, first, last)			\
	memcpy16(pal_obj_mem+(first), colors, (last)-(first)+1)
#define pal_obj_rotate(first, last)					\
	memrot16(pal_obj_mem+(first), colors, (last)-(first)+1)

// === INLINES ========================================================

// This one trunks input to correct sizes
INLINE COLOR RGB15(u8 rr, u8 gg, u8 bb)
{	return (rr&0x1f) | ((gg&0x1f)<<5) | ((bb&0x1f)<<10);	}

// This one doesn't, but is faster.
INLINE COLOR _RGB15(u8 rr, u8 gg, u8 bb)
{	return rr+(gg<<5)+(bb<<10);								}


#endif // GBA_COLOR

