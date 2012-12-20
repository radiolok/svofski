// affine.h: 
// affine functions
//
// (Created: 2003-04-06, Modified: 2004-04-28, Cearn)
// NOTE: subject to change

#ifndef GBA_AFFINE
#define GBA_AFFINE

#include "types.h"

// === CONSTANTS ======================================================

#define BGA_STRIDE  2   // P elements are continuous
#define OA_STRIDE   8   // P elements are 8 bytes apart

// === CLASSES ========================================================

// The affine transformation matrix P is defined as 	
// P= | pa pb |
//    | pc pd |
	
// NOTE: P describes the transformation from screen space to object space
//   so it is actually the inverse of what you're trying to accomplish.
// NOTE: the elements are 8.8 fixed numbers; fillers are necessary to 
//   account for the interlace with OBJINFO
	
// --- backgrounds ---

// BGAFF: P for backgrounds
typedef struct tagBGAFF
{
	s16 pa, pb;
	s16 pc, pd;
} BGAFF;

// BGAFF_EX: as BGAFF, but with displacement vector (dx, dy)
typedef struct tagBGAFF_EX
{
	s16 pa, pb;
	s16 pc, pd;
	s32 dx, dy;
} BGAFF_EX;

// --- Objects ---

// OBJAFF: P for objects
// elements are at 8 byte offsets (i.e., 3 u16 in between)
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


// --- structs for BIOS affine calls 0x0e and 0x0f ---
// simple scale-rotation 
typedef struct tagAFF_SRC
{
	s16 sx, sy;		// scales (8.8)
	u16 alpha;		// CCW angle ( range [0, 0xffff] )
} AFF_SRC;

// NOTE: these things are untested!
// scale-rotate around arbitrary point
typedef struct tagAFF_SRC_EX
{
	s32 px, py;		// vector p: origin in texture space (20.8)
	s16 qx, qy;		// vector q: origin in screen space (.0 ?)
	s16 sx, sy;		// scales (8.8)
	u16 alpha;		// CCW angle ( integer in [0,0xffff] )
} AFF_SRC_EX;


// === PROTOTYPES =====================================================

// --- OBJAFF functions ---

// screen -> object affine-transform functions
// basics
void oa_copy(OBJAFF *dest, OBJAFF *src);
void oa_transform(OBJAFF *oa, s16 ux, s16 uy, s16 vx, s16 vy);
void oa_identity(OBJAFF *oa);
void oa_postmultiply(OBJAFF *dest, OBJAFF *src);
// transforms
void oa_rotate(OBJAFF *oa, int alpha);
void oa_rotscale(OBJAFF *oa, int alpha, FIXED sx, FIXED sy);
void oa_rotscale2(OBJAFF *oa, AFF_SRC *as);
void oa_scale(OBJAFF *oa, FIXED sx, FIXED sy);
void oa_shearx(OBJAFF *oa, FIXED hx);
void oa_sheary(OBJAFF *oa, FIXED hy);

// inverse (object -> screen) functions, could be useful
// inverses (prototypes)
void oa_rotate_inv(OBJAFF *oa, int alpha);
void oa_rotscale_inv(OBJAFF *oa, int alpha, FIXED sx, FIXED sy);

INLINE void oa_scale_inv(OBJAFF *oa, FIXED sx, FIXED sy);
INLINE void oa_shearx_inv(OBJAFF *oa, FIXED hx);
INLINE void oa_sheary_inv(OBJAFF *oa, FIXED hy);

// --- BGAFF functions ---
// these do exactly the same as the functions given above, 
// but for backgrounds
// basics
void bga_copy(BGAFF *dest, BGAFF *src);
void bga_transform(BGAFF *bga, s16 ux, s16 uy, s16 vx, s16 vy);
void bga_identity(BGAFF *bga);
void bga_postmultiply(BGAFF *dest, BGAFF *src);
// transforms
void bga_rotscale(BGAFF *bga, int alpha, FIXED sx, FIXED sy);
void bga_rotate(BGAFF *bga, int alpha);
void bga_rotscale2(BGAFF *bga, AFF_SRC *as);
void bga_scale(BGAFF *bga, FIXED sx, FIXED sy);
void bga_shearx(BGAFF *bga, FIXED hx);
void bga_sheary(BGAFF *bga, FIXED hy);

// inverses (prototypes)
void bga_rotate_inv(BGAFF *bga, int alpha);
void bga_rotscale_inv(BGAFF *bga, int alpha, FIXED sx, FIXED sy);

INLINE void bga_scale_inv(BGAFF *bga, FIXED sx, FIXED sy);
INLINE void bga_shearx_inv(BGAFF *bga, FIXED hx);
INLINE void bga_sheary_inv(BGAFF *bga, FIXED hy);

// --- non-shared affine functions ---
void bga_rs_ex(BGAFF_EX *bgax, AFF_SRC_EX *asx);

// === INLINES=========================================================

// objects
INLINE void oa_scale_inv(OBJAFF *oa, FIXED sx, FIXED sy)
{	oa_scale(oa, (1<<16)/sx, (1<<16)/sy);	}

INLINE void oa_shearx_inv(OBJAFF *oa, FIXED hx)
{	oa_shearx(oa, -hx);						}

INLINE void oa_sheary_inv(OBJAFF *oa, FIXED hy)
{	oa_sheary(oa, -hy);						}

// backgrounds
INLINE void bga_scale_inv(BGAFF *bga, FIXED sx, FIXED sy)
{	bga_scale(bga, (1<<16)/sx, (1<<16)/sy);	}

INLINE void bga_shearx_inv(BGAFF *bga, FIXED hx)
{	bga_shearx(bga, -hx);					}

INLINE void bga_sheary_inv(BGAFF *bga, FIXED hy)
{	bga_sheary(bga, -hy);					}
#endif // GBA_AFFINE

