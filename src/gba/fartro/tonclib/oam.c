// oam.c: 
// source code for Object Attribute Memory stuff
// This is OBJINFO only; OBJAFF functions can be found in affine.c
//
// (Created: 2003-07-12, Modified: 2004-05-13, Cearn)

#include "types.h"
#include "core.h"
#include "oam.h"
#include "affine.h"

// === CONSTANTS ======================================================
// === GLOBALS ========================================================

OBJINFO oi_buffer[OI_COUNT];
OBJAFF *const oa_buffer= (OBJAFF*)oi_buffer;

// === PROTOTYPES =====================================================
// === FUNCTIONS =======================================================

// zero out all sprites and place them off the screen
void oam_init()
{
	int ii;
	for(ii=0; ii<OI_COUNT; ii++)
	{
		oi_buffer[ii].attr0= 160;
		oi_buffer[ii].attr1= 240;
		oi_buffer[ii].attr2= 0;
		oi_buffer[ii].fill= 0;
	}
	oam_update_all();
}

// copy oi_buffer to oam_mem
void oam_update(int start, int num)
{
	// num<<1 ?!? Remember, one OBJINFO struct is 4 u16 = 2 u32 long 
	dma_memcpy(&oam_mem[start], &oi_buffer[start], num<<1);
}

// update only the OBJINFO part
void oi_update(int start, int num)
{
	int ii;
	OBJINFO *oi= (OBJINFO*)oam_mem;
	for(ii=start; ii<start+num; ii++)
	{
		oi[ii].attr0= oi_buffer[ii].attr0;
		oi[ii].attr1= oi_buffer[ii].attr1;
		oi[ii].attr2= oi_buffer[ii].attr2;
	}
}

// update only the OBJAFF part
void oa_update(int start, int num)
{
	int ii;
	OBJAFF *oa= (OBJAFF*)oam_mem;
	for(ii=start; ii<start+num; ii++)
	{
		oa[ii].pa= oa_buffer[ii].pa;
		oa[ii].pb= oa_buffer[ii].pb;
		oa[ii].pc= oa_buffer[ii].pc;
		oa[ii].pd= oa_buffer[ii].pd;
	}
}

// --- OBJINFO functions ---

// NOTE: does x & OBJ_XMASK automatically takes care of neg values
void oi_set_pos(OBJINFO *oi, int x, int y)
{
	oi->attr1 &= ~OI_A1_XMASK;
	oi->attr1 |= (x & OI_A1_XMASK);
	
	oi->attr0 &= ~OI_A0_YMASK;
	oi->attr0 |= (y & OI_A0_YMASK);
}

void oi_hide(OBJINFO *oi)
{
	oi->attr1 &= ~OI_A1_XMASK;
	oi->attr1 |= 240;
	
	oi->attr0 &= ~OI_A0_YMASK;
	oi->attr0 |= 160;
}

// Don't optimize to *dest= *src plz. Why? 
// Cuz you'll f#$k up the interlaced OBJAFFs, that's why!
void oi_copy(OBJINFO *dest, OBJINFO *src)
{
	dest->attr0= src->attr0;
	dest->attr1= src->attr1;
	dest->attr2= src->attr2;
}


// EOF
