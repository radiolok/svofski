// oam.h: 
// header for Object Attribute Memory (OAM) stuff
//
// (Created: 2003-07-12, Modified: 2004-04-04, Cearn)

// (2004-04-04) I've renamed the whole thing yet again
// and I hope that this will be the last one. I'm 
// getting afwully tried of this

#ifndef GBA_OAM
#define GBA_OAM

#include "types.h"
#include "regs.h"
#include "affine.h"


// === CONSTANTS ======================================================

// number of OBJINFO structs (default: 128)
#ifndef OI_COUNT
#define OI_COUNT 128
#endif

// number of OBJAFF structs (default: 32)
#ifndef OA_COUNT
#define OA_COUNT 32
#endif

// --- attribute stuff ---

// attr0 flags
#define OI_A0_AFF_FLAG      0x0100
#define OI_A0_SIZE2X        0x0200
#define OI_A0_BLEND         0x0400
#define OI_A0_WIN           0x0800
#define OI_A0_MOS           0x1000
#define OI_A0_PAL256        0x2000
#define OI_A0_SQUARE        0x0000
#define OI_A0_WIDE          0x4000
#define OI_A0_TALL          0x8000

#define OI_A0_YMASK         0x00ff
#define OI_A0_SHAPE_MASK    0xc000
#define OI_A0_SHAPE_SHIFT   14

// attr1 flags
#define OI_A1_HFLIP         0x1000
#define OI_A1_VFLIP         0x2000
#define OI_A1_SIZE_8        0x0000
#define OI_A1_SIZE_16       0x4000
#define OI_A1_SIZE_32       0x8000
#define OI_A1_SIZE_64       0xc000

#define OI_A1_XMASK         0x01ff
#define OI_A1_AFF_MASK      0x3e00
#define OI_A1_SIZE_MASK     0xc000
#define OI_A1_AFF_SHIFT     9
#define OI_A1_SIZE_SHIFT    14

// attr2 flags
#define OI_A2_ID_MASK       0x03ff
#define OI_A2_PRIO_MASK     0x0c00
#define OI_A2_PAL16_MASK    0xf000

#define OI_A2_PRIO_SHIFT    10
#define OI_A2_PAL16_SHIFT   12

// === CLASSES ========================================================

// Object information
typedef struct tagOBJINFO
{
	u16 attr0;
	u16 attr1;
	u16 attr2;
	s16 fill;
}	OBJINFO;


// Object affine transformation matrix P
// P= | a b |
//    | c d |
// NOTE: the elements are 8.8 fixed numbers; fillers are necessary to 
//   account for the interlace with OBJINFO 
// NOTE: P describes the transformation from screen space to object space
//   so it is actually the inverse of what you're trying to accomplish.

// NOTE: this one's just for show; the real definition is in affine.h
#if 0
typedef struct tagOBJAFF
{
	u16 fill0[3];
	s16 pa;
	u16 fill1[3];
	s16 pb;
	u16 fill2[3];
	s16 pc;
	u16 fill3[3];
	s16 pd;
}	OBJAFF;
#endif

// === GLOBALS ========================================================

extern OBJINFO oi_buffer[];
extern OBJAFF * const oa_buffer;

// === PROTOTYPES =====================================================

// general oam functions
void oam_init();
void oam_update(int start, int num);

// --- OBJINFO functions ---
INLINE int oi_oam2aff(const OBJINFO *oi);
INLINE int oi_oam2prio(const OBJINFO *oi);
INLINE int oi_oam2pal(const OBJINFO *oi);

INLINE void oi_set_attr(OBJINFO *oi, u16 a0, u16 a1, u16 a2);
void oi_set_pos(OBJINFO *oi, int x, int y);
void oi_hide(OBJINFO *oi);
void oi_copy(OBJINFO *dest, OBJINFO *src);
void oi_update(int start, int num);

// --- OBJAFF functions ---
void oa_update(int start, int num);

// all transform-specific functions are in affine.c

// --- pseudo functions ---
// void oam_update_all();
// void oi_update_all();
// void oa_update_all();

// === MACROS ========================================================

#define oam_mem				((OBJINFO*)MEM_OAM)

#define oi_aff2oam(ii)  ( ((ii)<<OI_A1_AFF_SHIFT)   & OI_A1_AFF_MASK   )
#define oi_prio2oam(ii) ( ((ii)<<OI_A2_PRIO_SHIFT)  & OI_A2_PRIO_MASK  )
#define oi_pal2oam(ii)  ( ((ii)<<OI_A2_PAL16_SHIFT) & OI_A2_PAL16_MASK   )

// === INLINES ========================================================

INLINE int oi_oam2aff(const OBJINFO *oi)
{	return (oi->attr1 & OI_A1_AFF_MASK)>>OI_A1_AFF_SHIFT;	}

INLINE int oi_oam2prio(const OBJINFO *oi)
{	return (oi->attr2 & OI_A2_PRIO_MASK)>>OI_A2_PRIO_SHIFT;	}

INLINE int oi_oam2pal(const OBJINFO *oi)
{	return oi->attr2>>OI_A2_PAL16_SHIFT;			}

INLINE void oi_set_attr(OBJINFO *oi, u16 a0, u16 a1, u16 a2)
{	oi->attr0= a0;	oi->attr1= a1;	oi->attr2= a2;	}

// === PSEUDO FUNCTIONS ===============================================

#define oam_update_all()	oam_update(0, OI_COUNT)
#define oi_update_all()		oi_update(0, OI_COUNT)
#define oa_update_all()		oa_update(0, OA_COUNT)


#endif // GBA_OAM

//EOF
