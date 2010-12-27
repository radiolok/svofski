// vid.h
// video/screen stuff
//
// (Created: 2003-05-05, Modified: 2004-07-23, Cearn)

// NOTE: you CANNOT write to VRAM one byte at a time. Combine 
// two or more bytes to u16 or u32 and then write.

#ifndef GBA_VID
#define GBA_VID

#include "types.h"
#include "regs.h"

// === CONSTANTS ======================================================

// --- screen constants -----------------------------------
// mode 3,4 sizes:
#define VID_WIDTH       240
#define VID_HEIGHT      160
// mode 5 sizes
#define VID_WIDTH5      160
#define VID_HEIGHT5     128

// the total number of scanlines (including those in VBlank)
#define VID_LINES       228

#define VID_TILE_SIZE     8
#define VID_WIDTH_TILES  30
#define VID_HEIGHT_TILES 20
// or a bit shorter
#define SCR_W     VID_WIDTH
#define SCR_H     VID_HEIGHT
#define SCR_WT    VID_WIDTH_TILES
#define SCR_HT    VID_HEIGHT_TILES

// for page flipping:
#define VID_FLIP     0x0000a000

#define vid_mem			((u16*)MEM_VRAM)
#define vid_front_mem   ((u16*)MEM_VRAM)
#define vid_back_mem    ((u16*)(MEM_VRAM+VID_FLIP))

// --- tile(block) constants ------------------------------
// (aka characters, but that name would cause ambiguity)

// tile 8x8@4bpp: 32bytes; 8 ints
typedef struct { u32 data[8];  } TILE, TILE_S;
// tile block (512 tiles; 32x16 matrix)
typedef TILE TILEBLOCK[512];
typedef TILE TILEROW[32];
typedef TILE TILEMAT[16][32];

// same for 8bpp tiles (double sized tiles)
// If you have a better name than	TILE_D, please tell me.
typedef struct { u32 data[16]; } TILE_D;

typedef TILE_D TILEBLOCK_D[256];
typedef TILE_D TILEROW_D[16];
typedef TILE_D TILEMAT_D[16][16];

enum eTileBlocks
{
	TB_BG0=0, TB_BG1, TB_BG2, TB_BG3, 
	TB_OBJ, TB_OBJ_LO=TB_OBJ, TB_OBJ_HI
};

#define tl_mem          ( (TILEBLOCK*)MEM_VRAM     )
#define tl_mem_d        ( (TILEBLOCK_D*)MEM_VRAM   )
#define tl_obj_lomem    ( (TILE*)tl_mem[TB_OBJ_LO] )
#define tl_obj_himem    ( (TILE*)tl_mem[TB_OBJ_HI] )

// === REGISTER FLAGS =================================================

// Note:
//  (w)  : write-only bits
//  (r)  : read-only bits
//  (rw) : read/write. Default

// layer order:
#define LAYER_BG0       0x0001
#define LAYER_BG1       0x0002
#define LAYER_BG2       0x0004
#define LAYER_BG3       0x0008
#define LAYER_OBJ       0x0010

// --- REG_DISPCNT (0400:0000) ----------------------------

// video mode 0-5
#define VID_MODE0       0x0000
#define VID_MODE1       0x0001
#define VID_MODE2       0x0002
#define VID_MODE3       0x0003
#define VID_MODE4       0x0004
#define VID_MODE5       0x0005
#define VID_MODE_MASK   0x0007
// misc
#define VID_GB          0x0008  // (r) indicates a GB rom (iso GBA)
#define VID_PAGE        0x0010	// back-page active
#define VID_OAM_UNLOCK  0x0020	// unlock OAM for writing during HBlank
#define VID_LINEAR      0x0040	// linear tile mapping
#define VID_BLANK       0x0080	// force screen blank.
// layer enables
#define VID_BG0         (LAYER_BG0<<8)
#define VID_BG1         (LAYER_BG1<<8)
#define VID_BG2         (LAYER_BG2<<8)
#define VID_BG3         (LAYER_BG3<<8)
#define VID_OBJ         (LAYER_OBJ<<8)
// window enables
#define VID_WIN0        0x2000
#define VID_WIN1        0x4000
#define VID_WINOBJ      0x8000

// --- REG_DISPSTAT (0400:0004) ---------------------------

// blank period status bits
#define VID_STAT_VB		0x0001	// (r)
#define VID_STAT_HB		0x0002	// (r)
#define VID_STAT_VC		0x0004	// (r)
// video interrupt requests
#define VID_IRQ_VB		0x0008
#define VID_IRQ_HB		0x0010
#define VID_IRQ_VC		0x0020
// VCount irq condition mask
#define VID_CND_MASK    0xff00

// --- REG_BLDMOD (0400:0050) -----------------------------

#define BLD_BG0			LAYER_BG0
#define BLD_BG1			LAYER_BG1
#define BLD_BG2			LAYER_BG2
#define BLD_BG3			LAYER_BG3
#define BLD_OBJ			LAYER_OBJ
#define BLD_BD			0x0020

// top layer
#define BLD_TOP(layer)  (layer)
// modes
#define BLD_NULL		0x0000
#define BLD_STD			0x0040
#define BLD_WHITE		0x0080
#define BLD_BLACK		0x00c0
// bottom layer
#define BLD_BOT(layer)  ((layer)<<8)


// --- windowing control (0400:0048 and :004a) --------------
#define WIN_BG0			LAYER_BG0
#define WIN_BG1			LAYER_BG1
#define WIN_BG2			LAYER_BG2
#define WIN_BG3			LAYER_BG3
#define WIN_OBJ			LAYER_OBJ
#define WIN_BLD			0x0020


// === GLOBALS ========================================================

extern u16* vid_page;		// current video page

// === PROTOTYPES =====================================================
// ("What is this, some kind of weapon?"
//  "Put that down; it's a prototype."
//  *pshew* *BOOM* !!
//  "Man, why aren't we using this thing?"
//  "It's much too unpredictable. Don't let it overcharge!"
//  "Wha, whaddya mean `overcharge'?"
//  "UUUUUAAARRRGGGHHHHLLL!!" *BOOM*
// )

// set full control
INLINE void vid_cnt(u16 mode, u16 layer, u16 win, u16 flags);
// just the mode
INLINE void vid_set_mode(u16 mode);

INLINE u32 vid_is_vblank();				// inside vblank?
INLINE void vid_irq_cond(u8 vcount);	// set the vc irq condition

// if this doesn't work, you've done something very wrong
void vid_sanity();
// plotting routines
// - unsafe, but fast
INLINE void _vid_plot8(int x, int y, u8 clr);
INLINE void _vid_plot16(int x, int y, COLOR clr);
// misc
u16* vid_flip();
INLINE void vid_wait_for_vblank();
INLINE void vid_wait_for_vdraw();
INLINE void vid_vsync();
void vid_wait(int count);

// --- gfx ------------------------------------------------
// mosaic
INLINE void vid_mosaic(u8 bh, u8 bv, u8 oh, u8 ov);

// blend
INLINE void bld_cnt(u16 top, u16 bot, u16 mode);
INLINE void bld_cnt_top(u16 top);
INLINE void bld_cnt_mode(u16 mode);
INLINE void bld_cnt_bottom(u16 bot);
INLINE void bld_set_weights(u8 top, u8 bot);
INLINE void bld_set_fade(u8 fade);

// --- windowing ---
void win_init(u16 wins);
// set window rectangle
INLINE void win0_set_rect(u8 l, u8 t, u8 r, u8 b);
INLINE void win1_set_rect(u8 l, u8 t, u8 r, u8 b);
// set window control flags
INLINE void win_in_cnt(u8 mode0, u8 mode1);
INLINE void win0_cnt(u8 mode);
INLINE void win1_cnt(u8 mode);
INLINE void win_out_cnt(u8 mode);
INLINE void win_obj_cnt(u8 mode);

// === MACROS =========================================================

// === INLINES ========================================================

INLINE void vid_cnt(u16 mode, u16 layer, u16 win, u16 flags)
{	REG_DISPCNT= (win&0xe000) | (layer&0x1f00) | (mode&7) | flags;	}

INLINE void vid_set_mode(u16 mode)
{	REG_DISPCNT &= ~7;	REG_DISPCNT |= mode&7;	}

INLINE void vid_irq_cond(u8 vcount)
{	REG_DISPSTAT &= 0x00ff;	REG_DISPSTAT |= vcount<<8;	}

// unsafe plotting
// mode 4 (yes, I know this fills two pixels)
INLINE void _vid_plot8(int x, int y, u8 clr)
{	((u8*)vid_page)[x + y*VID_WIDTH]= clr;	}

// mode 3, 5
INLINE void _vid_plot16(int x, int y, COLOR clr)
{	vid_page[x + y*VID_WIDTH]= clr;			}

// vdraw/vblank stuff
INLINE u32 vid_is_vblank()
{	return REG_DISPSTAT & VID_STAT_VB;				}

INLINE void vid_wait_for_vblank()
{	while(REG_VCOUNT < VID_HEIGHT);					}

INLINE void vid_wait_for_vdraw()
{	while(REG_VCOUNT >= VID_HEIGHT);				}

// wait till the _next_ vblank
// Note that for real work, using a VBlank interrupt is preferred
INLINE void vid_vsync()
{	vid_wait_for_vdraw();	vid_wait_for_vblank();	}

// --- GRAPHIC EFFECTS ------------------------------------

// --- mosaic ---
INLINE void vid_mosaic(u8 bh, u8 bv, u8 oh, u8 ov)
{	REG_MOSAIC= bh + (bv<<4) + (oh<<8) + (ov<<12);		}

// --- blending ---
INLINE void bld_cnt(u16 top, u16 bot, u16 mode)
{	REG_BLDMOD= (top&0x3f) | (bot<<8) | mode;			}

INLINE void bld_cnt_top(u16 top)
{	REG_BLDMOD &= ~0x003f;	REG_BLDMOD |= top;			}

INLINE void bld_cnt_mode(u16 mode)
{	REG_BLDMOD &= ~0x00c0;	REG_BLDMOD |= mode;			}

INLINE void bld_cnt_bottom(u16 bot)
{	REG_BLDMOD &= ~0x3f00;	REG_BLDMOD |= (bot<<8);		}


INLINE void bld_set_weights(u8 top, u8 bot)
{	REG_COLEV= (top&0x1f) | (bot<<8);					}

INLINE void bld_set_fade(u8 fade)
{	REG_COLEY= fade;										}

// --- windowing ---

// set window boundaries: 
// left, top, right, bottom
INLINE void win0_set_rect(u8 l, u8 t, u8 r, u8 b)
{	REG_WIN0H= (l<<8) | r; 	REG_WIN0V= (t<<8) | b;		}

INLINE void win1_set_rect(u8 l, u8 t, u8 r, u8 b)
{	REG_WIN1H= (l<<8) | r;	REG_WIN1V= (t<<8) | b;		}

// set window control flags
INLINE void win_in_cnt(u8 mode0, u8 mode1)
{	REG_WININ= (mode1<<8) | mode0;						}

INLINE void win0_cnt(u8 mode)
{	REG_WININ &= 0xff00;	REG_WININ |= mode;			}

INLINE void win1_cnt(u8 mode)
{	REG_WININ &= 0x00ff;	REG_WININ |= (mode<<8);		}

INLINE void win_out_cnt(u8 mode)
{	REG_WINOUT &= 0xff00;	REG_WINOUT |= mode;			}

INLINE void win_obj_cnt(u8 mode)
{	REG_WINOUT &= 0x00ff;	REG_WINOUT |= (mode<<8);	}

#endif // GBA_VID


