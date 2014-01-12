// bg.h: 
// background header file
// NOTE! Still very much a work in progress
//
// (Created: 2003-05-23, Modified: 2004-07-16, Cearn)

#ifndef GBA_BG
#define GBA_BG

#include "types.h"
#include "vid.h"
#include "affine.h"
#include "geom.h"

// === CONSTANTS ======================================================

#define BG_MAX                   4
#define BG_TB_SIZE          0x4000
#define BG_MAP_SIZE         0x0800

// REG_BGxCNT
#define BG_PRIO_MASK        0x0003
#define BG_TBB_MASK         0x000c
#define BG_TBB_SH                2
#define BG_MOS              0x0040
#define BG_PAL256           0x0080
#define BG_MBB_MASK         0x1f00
#define BG_MBB_SH                8
#define BG_AFF_WRAP         0x2000
#define BG_SIZE_MASK        0xc000
#define BG_SIZE_SH              14

// size options
#define BG_REG_1X1          0x0000
#define BG_REG_2X1          0x4000
#define BG_REG_1X2          0x8000
#define BG_REG_2X2          0xc000
// same thing, only different (and longer) names
#define BG_REG_256X256      0x0000
#define BG_REG_512X256      0x4000
#define BG_REG_256X512      0x8000
#define BG_REG_512X512      0xc000

#define BG_AFF_128          0x0000
#define BG_AFF_256          0x4000
#define BG_AFF_512          0x8000
#define BG_AFF_1024         0xc000


// screen map flags
#define BG_ID_MASK          0x03ff
#define BG_HFLIP            0x0400
#define BG_VFLIP            0x0800
#define BG_PAL16_MASK       0xf000
#define BG_PAL16_SH         12


// === CLASSES ========================================================

typedef TEGEL MAPBLOCK[32*32];
typedef TEGEL MAPROW[32];
typedef TEGEL MAPMAT[32][32];

// BGAFF and BGAFF_EX structs are defined in affine.h
// These are just for show
//typedef struct tagBGAFF
//{
//	s16 pa, pb, pc, pd;
//} BGAFF;
//
//typedef struct tagBGAFF_EX
//{
//	s16 pa, pb, pc, pd;
//	s32 dx, dy;
//} BGAFF_EX;

typedef struct tagBGINFO
{
	u16 nr;
	u16 reg;
	union { TILE *tiles; TILEBLOCK *tb;	};
	union { TEGEL *map;  MAPBLOCK *mb;	};
	BOOL bAffine;
//	u16 w;
//	u16 h;
	BGAFF bga;	// can be cast to BGAFF_EX for easy copy to registers
	s32 dx;		// should be a union, I know, but not sure how to do 
	s32 dy;		// that :(
} BGINFO;

typedef struct { u16 hofs,vofs; } BGPOINT;

// some base pointers
#define bg_cnt_mem    (    (vu16*)0x04000008)
#define bg_ofs_mem    ( (BGPOINT*)0x04000010)	// (write only!)
#define bga_ex_mem    ((BGAFF_EX*)0x04000000)	// (write only!)
#define map_mem       ((MAPBLOCK*)0x06000000)

// === GLOBALS ========================================================

// === PROTOTYPES =====================================================


// --- BGINFO functions ---
void bg_init(BGINFO *bg, int bgnr, int tb_base, int map_base, u16 flags);
void bg_update(const BGINFO *bg);
void bga_post_translate(BGINFO *bg, s32 x, s32 y);
INLINE void bg_set_pos(BGINFO *bg, FIXED dx, FIXED dy);
int bg_tegel_id(BGINFO *bg, u16 tx, u16 ty);

INLINE void bg_rotscale(BGINFO *bg, AFF_SRC_EX *asx);
INLINE void bga_ex_update(const BGINFO *bg);

// --- map functions ---
void map_fill(MAPBLOCK *mb, TEGEL tegel);
void map_fill_line(MAPBLOCK *mb, int line, TEGEL tegel);
void map_fill_rect(MAPBLOCK *mb, const RECT *rc, TEGEL tegel);

// === MACROS ========================================================

#define bg_mbb2cnt(ii)	( ((ii) << BG_MBB_SH) & BG_MBB_MASK )
#define bg_tbb2cnt(ii)	( ((ii) << BG_TBB_SH) & BG_TBB_MASK )
#define bg_pal2cnt(ii)	(  (ii) << BG_PAL16_SH )

#define bg_cnt2mbb(reg)	( ((ii) & BG_MBB_MASK) >> BG_MBB_SH )
#define bg_cnt2tbb(reg)	( ((ii) & BG_TBB_MASK) >> BG_TBB_SH )
#define bg_cnt2pal(reg)	( ((ii) >> BG_PAL16_SH )

// === INLINES ========================================================

INLINE void bg_set_pos(BGINFO *bg, FIXED dx, FIXED dy)
{	bg->dx= dx;  bg->dy= dy;						}

INLINE void bg_rotscale(BGINFO *bg, AFF_SRC_EX *asx)
{	bga_rs_ex((BGAFF_EX*)(&bg->bga), asx);			}

INLINE void bga_ex_update(const BGINFO *bg)
{	bga_ex_mem[bg->nr]= *(BGAFF_EX*)(&bg->bga);		}

#endif // GBA_BG

