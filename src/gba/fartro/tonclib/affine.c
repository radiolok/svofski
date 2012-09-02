// affine.c: 
// source code for Affine Transformation stuff
//
// (Created: 2003-10-03, Modified: 2004-07-16, Cearn)

#include "types.h"
#include "affine.h"
//#include "oam.h"
#include "luts.h"

// ====================================================================
//
// On affine transformations and the GBA matrix P
//
// There are three affine (line-preserving) transformations: 
// rotation R, scaling S and shear H. These are defined as
//
// R(a) = | cos(a)  -sin(a) |   R^1(a) = R(-a)
//        | sin(a)   cos(a) |
//
// S(sx, sy) = | sx   0 |		S^1(sx, sy) = S(1/sx, 1/sy)
//             |  0  sy |
// 
// H(hx, hy) = |  1  hx |		H^1(hx, hy) = H(-hx, -hy)/(1-hx*hy)
//             | hy   1 |
//
// you could simply do, say, P= S, but you might not get what 
// you expected. The thing is that P maps from screen space to
// object/bg space, while people tend think the other way round.
// see http://user.chem.tue.nl/jakvijn/tonc/affine.htm for details
// 
// The functions below see things the GBA way, so you'd need account for
// inverted parameters yourself. If you don't feel like doing so you
// could always use the functions with the "_inv" extension, which will do 
// the invertions for you, but it'll cost ya in speed.
//
// ====================================================================

// Since BGAFF and OBJAFF are  identical except for the stride of the 
// P elements, I'm using macros to cover them both. They have to be 
// macros, because although the code is the (written) code would 
// be the same for the oa_ and bga_ functions, you can't just swap 
// BGAFF and OBJAFF structs because of the strides. I've tried to combine
// the two into something like
//
// <code>
// #define obj_foo(poa, x, y, z) aff_foo(&(pmat->pa), x, y, z, 1)
// #define bg_foo(pbga, x, y, z) aff_foo((u16*)(pbga), x, y, z, 4)
//
// aff_foo(s16 *mat, int x, int y, int z, int stride)
// {
//    ...
// }
// </code>
//
// But that doesn't work. But I'll be damned if I let anything like 
// that stop me.
// I think it'd work in pure assembly, though.

// === CODE MACROS ===
// In all _aff_xxx macros, mat, src, and dest are P matrices


// copies src into dest
#define _aff_copy(dest, src)	\
{								\
	dest->pa= src->pa;			\
	dest->pb= src->pb;			\
	dest->pc= src->pc;			\
	dest->pd= src->pd;			\
}

// sets mat to u and v vectors
#define _aff_transform(mat, ux, uy, vx, vy)		\
{												\
	mat->pa= ux;								\
	mat->pb= vx;								\
	mat->pc= uy;								\
	mat->pd= vy;								\
}

// sets mat to identity 
#define _aff_identity(mat)		\
{								\
	mat->pa= 1<<8;				\
	mat->pb= mat->pc= 0;		\
	mat->pd= 1<<8;				\
}

// dest= dest*src
// postmultiplication because we are working in reverse order
// (P is an inverse matrix, remember?)
#define _aff_postmultiply(dest, src)					\
{														\
	FIXED tmp_a, tmp_b, tmp_c, tmp_d;					\
	tmp_a= dest->pa; tmp_b= dest->pb;					\
	tmp_c= dest->pc; tmp_d= dest->pd;					\
	dest->pa= (tmp_a*src->pa + tmp_b*src->pc)>>8;		\
	dest->pb= (tmp_a*src->pb + tmp_b*src->pd)>>8;		\
	dest->pc= (tmp_c*src->pa + tmp_d*src->pc)>>8;		\
	dest->pd= (tmp_c*src->pb + tmp_d*src->pd)>>8;		\
}

// --- screen -> object transformations ---
// rotates CCW by alpha
#define _aff_rotate(mat, alpha)		\
{									\
	mat->pa= lut_cos(alpha);		\
	mat->pc= lut_sin(alpha);		\
	mat->pb= -mat->pc;				\
	mat->pd= mat->pa;				\
}

// scales by 1/sx, 1/sy _Then_ rotates CCW by alpha
#define _aff_rotscale(mat, alpha, sx, sy)	\
{											\
	mat->pa=  (lut_cos(alpha)*sx)>>8;		\
	mat->pb= -(lut_sin(alpha)*sx)>>8;		\
	mat->pc=  (lut_sin(alpha)*sy)>>8;		\
	mat->pd=  (lut_cos(alpha)*sy)>>8;		\
}

// ditto, but using AFF_SRC for source parameters
// UNTESTED!
#define _aff_rotscale2(mat, as)					\
{	int alpha= as->alpha; /*>> 7;	cut down to [0,511] range */	\
	mat->pa=  (lut_cos(alpha)*as->sx)>>8;		\
	mat->pb= -(lut_sin(alpha)*as->sx)>>8;		\
	mat->pc=  (lut_sin(alpha)*as->sy)>>8;		\
	mat->pd=  (lut_cos(alpha)*as->sy)>>8;		\
}

// scale by 1/sx and 1/sy
#define _aff_scale(mat, sx, sy)		\
{									\
	mat->pa= sx;					\
	mat->pb= mat->pc= 0;			\
	mat->pd= sy;					\
}

// shear by -hx
#define _aff_shearx(mat, hx)		\
{									\
	mat->pa= 1<<8;					\
	mat->pb= hx;					\
	mat->pc= 0;						\
	mat->pd= 1<<8;					\
}

// shear by -hy
#define _aff_sheary(mat, hy)		\
{									\
	mat->pa= 1<<8;					\
	mat->pb= 0;						\
	mat->pc= hy;					\
	mat->pd= 1<<8;					\
}

// object -> screen transformations
// rotate CW by alpha
#define _aff_rotate_inv(mat, alpha)			\
{											\
	mat->pd= mat->pa= lut_cos(alpha);		\
	mat->pc= -(mat->pb= lut_sin(alpha));	\
}

// scale by sx, sy, _then_ rotate by alpha
#define _aff_rotscale_inv(mat, alpha, sx, sy)	\
{												\
	FIXED trig_tmp= lut_cos(alpha);				\
	/* .12 fixed */								\
	FIXED _sx=  (1<<20)/sx, _sy= (1<<20)/sy;	\
												\
	mat->pa=   (trig_tmp*_sx)>>12;				\
	mat->pd=   (trig_tmp*_sy)>>12;				\
												\
	trig_tmp= lut_sin(alpha);					\
	mat->pb=   (trig_tmp*_sx)>>12;				\
	mat->pc=  -(trig_tmp*_sy)>>12;				\
}

// === WRAPPER FUNCTIONS ==============================================

// --- OBJ wrappers ---
void oa_copy(OBJAFF *dest, OBJAFF *src)
	_aff_copy(dest, src)

void oa_transform(OBJAFF *oa, s16 ux, s16 uy, s16 vx, s16 vy)
	_aff_transform(oa, ux, uy, vx, vy)

void oa_identity(OBJAFF *oa)
	_aff_identity(oa)

void oa_postmultiply(OBJAFF *dest, OBJAFF *src)
	_aff_postmultiply(dest, src)

void oa_rotscale(OBJAFF *oa, int alpha, FIXED sx, FIXED sy)
	_aff_rotscale(oa, alpha, sx, sy)

void oa_rotate(OBJAFF *oa, int alpha)
	_aff_rotate(oa, alpha )

void oa_rotscale2(OBJAFF *oa, AFF_SRC *as)
	_aff_rotscale2(oa, as)

void oa_scale(OBJAFF *oa, FIXED sx, FIXED sy)
	_aff_scale(oa, sx, sy)

void oa_shearx(OBJAFF *oa, FIXED hx)
	_aff_shearx(oa, hx)

void oa_sheary(OBJAFF *oa, FIXED hy)
	_aff_sheary(oa, hy)

// inverse (object -> screen) functions, could be useful
void oa_rotate_inv(OBJAFF *oa, int alpha)
	_aff_rotate_inv(oa, alpha)	

void oa_rotscale_inv(OBJAFF *oa, int alpha, FIXED sx, FIXED sy)
	_aff_rotscale_inv(oa, alpha, sx, sy)


// --- BG wrappers ---
void bga_copy(BGAFF *dest, BGAFF *src)
	_aff_copy(dest, src)

void bga_transform(BGAFF *bga, s16 ux, s16 uy, s16 vx, s16 vy)
	_aff_transform(bga, ux, uy, vx, vy)

void bga_identity(BGAFF *bga)
	_aff_identity(bga)

void bga_postmultiply(BGAFF *dest, BGAFF *src)
	_aff_postmultiply(dest, src)

void bga_rotscale(BGAFF *bga, int alpha, FIXED sx, FIXED sy)
	_aff_rotscale(bga, alpha, sx, sy)

void bga_rotate(BGAFF *bga, int alpha)
	_aff_rotate(bga, alpha )

void bga_rotscale2(BGAFF *bga, AFF_SRC *as)
	_aff_rotscale2(bga, as)

void bga_scale(BGAFF *bga, FIXED sx, FIXED sy)
	_aff_scale(bga, sx, sy)

void bga_shearx(BGAFF *bga, FIXED hx)
	_aff_shearx(bga, hx)

void bga_sheary(BGAFF *bga, FIXED hy)
	_aff_sheary(bga, hy)

// inverse (bg -> screen) functions, could be useful
void bga_rotate_inv(BGAFF *bga, int alpha)
	_aff_rotate_inv(bga, alpha)	

void bga_rotscale_inv(BGAFF *bga, int alpha, FIXED sx, FIXED sy)
	_aff_rotscale_inv(bga, alpha, sx, sy)

void bga_rs_ex(BGAFF_EX *bgax, AFF_SRC_EX *asx)
{
	int alpha= asx->alpha; /* >> 7;	cut down to [0,511] range */
	BGAFF bga;
	bga.pa=  (lut_cos(alpha)*asx->sx)>>8;
	bga.pb= -(lut_sin(alpha)*asx->sx)>>8;
	bga.pc=  (lut_sin(alpha)*asx->sy)>>8;
	bga.pd=  (lut_cos(alpha)*asx->sy)>>8;
	*(BGAFF*)bgax= bga;
	bgax->dx= asx->px - (bga.pa*asx->qx + bga.pb*asx->qy);
	bgax->dy= asx->py - (bga.pc*asx->qx + bga.pd*asx->qy);
}

/*
// No matter what you do, combining bg and obj affine functions 
// into one (like swi 0x0e and 0x0f do) will never work in C
// The assembly code of this is 50 lines, against 28  of the original 
// oa_rotscale. Clearly, I'm being too clever for my own good.
// (In pure assembly it might work, though)
void aff_rotscale(s16 *mat, OBJAFF_SRC *os, int offset)
{
	*mat=  (lut_cos(os->alpha)*os->sx)>>8;
	mat += offset;
	*mat= -(lut_sin(os->alpha)*os->sx)>>8;
	mat += offset;
	*mat=  (lut_sin(os->alpha)*os->sy)>>8;
	mat += offset;
	*mat=  (lut_cos(os->alpha)*os->sy)>>8;
	mat += offset;
}
*/

// === REAL FUNCTIONS ===



// EOF
