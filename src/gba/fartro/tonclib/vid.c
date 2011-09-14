// 
// vid.c: video functions
//
// (Created: 2003-05-05, Modified: 2004-01-06, Cearn)

#include "vid.h"
#include "color.h"

// === GLOBALS ========================================================

u16 *vid_page= vid_front_mem;	// vbuf starts as 0x06000000

// === FUNCTIONS ======================================================

void vid_sanity()
{
	REG_DISPCNT = VID_MODE3|VID_BG2;
	// top left pixels:
	_vid_plot16(16, 16, REDMASK);
	_vid_plot16(32, 16, GREENMASK);
	_vid_plot16(16, 32, BLUEMASK);
}

u16* vid_flip()
{
	// <cld>
	// yes it looks a hackish, but this is a perfectly xorrable 
	// operation and I'll be damned if I let something like 
	// the fact that C won't let me xor pointers stop me!
	// </cld>
	vid_page= (u16*)((u32)vid_page ^ VID_FLIP);
	REG_DISPCNT ^= VID_PAGE;			// update control register	

	return vid_page;
}

void vid_wait(int count)
{
	u16 vline= REG_VCOUNT;
	while(count-- > 0)
	{
		while(vline == REG_VCOUNT);
		while(vline != REG_VCOUNT);
	}
}

void win_init(u16 wins)
{
	REG_DISPCNT |= wins;
	REG_WIN0H=  VID_WIDTH;
	REG_WIN1H=  VID_WIDTH;
	REG_WIN0V=  VID_HEIGHT;
	REG_WIN1V=  VID_HEIGHT;

	REG_WININ= 0x3f3f;
}
