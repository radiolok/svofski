// bg.c: 
// background source file
//
// (Created: 2003-05-23, Modified: 2004-07-23, Cearn)

#include "types.h"
#include "vid.h"
#include "affine.h"
#include "bg.h"

// === CONSTANTS ======================================================
// === GLOBALS ========================================================
// === PROTOTYPES =====================================================
// === INLINES ========================================================
// === FUNCTIONS ======================================================

void bg_init(BGINFO *bg, int bgnr, int tbb, int mbb, u16 flags)
{
	tbb &= 3;
	mbb &= 31;

	bg->nr= bgnr&3;
	bg->reg= flags;
	bg->reg |= bg_tbb2cnt(tbb);
	bg->reg |= bg_mbb2cnt(mbb);

	// point to correct tile-block and map data
	bg->tiles= tl_mem[tbb];
	bg->map= map_mem[mbb];

	// check for affine-bg
	u16 vmode= REG_DISPCNT&VID_MODE_MASK;
	if( (bgnr == 2 && (vmode==VID_MODE1 || vmode==VID_MODE2) )
		|| (bgnr == 3 && (vmode == VID_MODE2)) )
	{
		bg->bAffine= TRUE;
		bg->bga.pa= bg->bga.pd= 0x0100;
		bg->bga.pb= bg->bga.pc= 0;
	}
	else
		bg->bAffine= FALSE;
}

void bg_update(const BGINFO *bg)
{
	bg_cnt_mem[bg->nr]= bg->reg;
	if(bg->bAffine)
	{
		bga_ex_update(bg);
	}
	else
	{
		bg_ofs_mem[bg->nr].hofs= (u16)bg->dx;
		bg_ofs_mem[bg->nr].vofs= (u16)bg->dy;
	}
}

int bg_tegel_id(BGINFO *bg, u16 tx, u16 ty)
{
    u16 n= tx + (ty<<5);
    if(tx >= 32)
        n += 0x03e0;
    if(ty >= 32 && (bg->reg&BG_REG_2X2) )
        n += 0x0400;
    return n;
}

// to affine.c ?
void bga_post_translate(BGINFO *bg, s32 x, s32 y)
{
	bg->dx= (bg->bga.pa*x + bg->bga.pb*y)>>8;
	bg->dy= (bg->bga.pc*x + bg->bga.pd*y)>>8;
}

// --- map functions --------------------------------------------------
void map_fill(MAPBLOCK *mb, TEGEL tegel)
{
	int ii;
	u32 *map= (u32*)mb, tgl32= tegel | (tegel<<16);
	for(ii=0; ii<sizeof(MAPBLOCK)>>2; ii++)
		map[ii]= tgl32;
}

void map_fill_line(MAPBLOCK *mb, int line, TEGEL tegel)
{
	int ii;
	u32 *map= (u32*)(&mb[0][line<<5]), tgl32= tegel | (tegel<<16);	
	for(ii=0; ii<16; ii++)
		map[ii]= tgl32;
}

void map_fill_rect(MAPBLOCK *mb, const RECT *rc, TEGEL tegel)
{
	int ix, iy;
	MAPROW *mr= (MAPROW*)mb;
	for(iy=rc->t; iy<rc->b; iy++)
		for(ix=rc->l; ix<rc->r; ix++)
			mr[iy][ix]= tegel;

}
