// swi.h: 
// header file for BIOS calls 
//
// (Created: 2004-03-24, Modified: 2004-07-06, Cearn)
// NOTE: very much a work in progress

#ifndef GBA_SWI
#define GBA_SWI

#include "types.h"
#include "affine.h"	// for affine structs


// === CONSTANTS =====================================================

// === MACROS =========================================================

// switch this lets you switch between ARM and THUMB models with 
// ease: __thumb__ is a compiler constant that is defined when, 
// what else, you are in thumb state
// There is a catch, of course. The catch is that the macro argument 
// MUST be in hex (since it's inserted literally)

// Taken from Wintermute's libgba (www.devkit.tk)
// NOTE: not all the BIOS calls use all the registers, so 
// this structure might be a little on the safe side
#if	defined	( __thumb__ )
#define	swi_call(x)	 asm volatile("swi\t"#x ::: "r0", "r1", "r2", "r3")
#else
#define	swi_call(x)	 asm volatile("swi\t"#x"<<16" ::: "r0", "r1", "r2", "r3")
#endif


// === CLASSES ========================================================

// --- affine source and destination structs ---
// for affine structs, see affine.h
// sources
typedef AFF_SRC	    ObjAffineSource;
typedef AFF_SRC_EX  BGAffineSource;
// destinations
typedef BGAFF		ObjAffineDest;
typedef BGAFF_EX	BGAffineDest;

// --- unpack struct ---
#define UPI_ZERO_FLAG 0x80000000

typedef struct tagUNPACKINFO
{
	u16 src_len;
	u8 src_bpp;
	u8 dst_bpp;
	u32 dst_offset;
} UNPACKINFO;

// === PROTOTYPES =====================================================

// swi_reset (swi 0)
// Note that this needs
// a) REG_IME=0 and 
// b) 0x0300:7ffa =0 for ROM boot; =!0 for multiboot
void swi_reset();

// wait for VBlank interrupt (swi 0x05)
void swi_vsync();

// returns num/denom; uses swi 0x06
// NOTE! div by 0 would fit extremely well in the Bad Things category
int swi_div(int num, int denom);

// as swi_div, but catches 0-division badness
int swi_div_safe(int num, int denom);

// returns num%denom
int swi_mod(int num, int denom);

// returns sqrt(num); uses swi 0x08
int swi_sqrt(u32 num);

// returns atan(y/x); uses swi 0x0a, Don't forget the s16!!
s16 swi_arctan2(s16 x, s16 y);

 
//                            | sx*cos(alpha)   -sx*sin(alpha) |
//  P = S(sx,sy)·R(alpha)   = |                                |
//                            | sy*sin(alpha)    sy*cos(alpha) |
// dx = p - P·q. The rotation point is map-point p and screen-point q. 
// alpha's range is [0,0xffff]
// uses swi 0x0e
// UNTESTED
void swi_aff_ex(AFF_SRC_EX *src, BGAFF_EX *dst);

// P = S(sx,sy)*R(alpha)
// in other words, does exactly what _aff_rotscale does (except for 
// the angle range). The destination is a pointer to a pa element, 
// the offset says how far apart the elements are 
// (use BG_AFF_OFS and OBJ_AFF_OFS)
// uses swi 0x0f
void swi_aff(AFF_SRC *src, void *dst, int num, int offset);


// Bit unpacking; unpacks between bit-depths
void swi_bitunpack(const void *src, void *dst, UNPACKINFO *upi);


// === 'official' BIOS Call names =====================================
// since my BIOS call naming scheme differs a little from the convential 
// one, here's a list of defines so you can use the standard names 
// too

#define VBlankIntrWait  swi_vsync
#define Div             swi_div
#define Sqrt            swi_sqrt
#define ArcTan2         swi_arctan2
#define BgAffineSet     swi_aff_ex
#define ObjAffineSet    swi_aff
#define BitUnPack       swi_bitunpack

#endif // GBA_SWI

