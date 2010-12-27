// geom.h: 
// geometry header file
//
// (Created: 2003-06-16, Modified: 2004-07-08, Cearn)

#ifndef GBA_GEOM
#define GBA_GEOM

#include "types.h"

// === CONSTANTS ======================================================
// === CLASSES ========================================================

typedef struct tagPOINT
{
	int x, y;
} POINT;

typedef struct tagRECT
{
	int l, t, r, b;
} RECT;

typedef struct tagVECTOR
{
	FIXED x, y, z;
} VECTOR;


// === PROTOTYPES =====================================================

// --- point ---
INLINE void pt_set(POINT *a, int x, int y);
INLINE void pt_add(POINT *dst, const POINT *a, const POINT *b);
INLINE void pt_scale(POINT *dst, const POINT *a, int c);
INLINE BOOL pt_in_rect(const POINT *pt, const RECT *rc);

// --- rect ---
INLINE void rc_set(RECT *rc, int l, int t, int r, int b);
INLINE void rc_set2(RECT *rc, int x, int y, int w, int h);
INLINE int rc_width(const RECT *rc);
INLINE int rc_height(const RECT *rc);
void rc_pos(RECT *rc, int x, int y);
INLINE void rc_size(RECT *rc, int w, int h);
INLINE void rc_move(RECT *rc, int dx, int dy);
INLINE void rc_inflate(RECT *rc, int dw, int dh);
INLINE void rc_inflate2(RECT *rc, const RECT *dr);

void rc_norm(RECT *rc);

// --- vector ---
INLINE void vec_set(VECTOR *a, FIXED x, FIXED y, FIXED z);

void vec_add(VECTOR *res, const VECTOR *a, const VECTOR *b);
void vec_sub(VECTOR *dst, const VECTOR *a, const VECTOR *b);
void vec_scale(VECTOR *dst, const VECTOR *a, FIXED c);
FIXED vec_dot(const VECTOR *a, const VECTOR *b);
void vec_cross(VECTOR *dst, const VECTOR *a, const VECTOR *b);

// === INLINES=========================================================

// --- point ---
INLINE void pt_set(POINT *a, int x, int y)
{	a->x= x;	a->y= y;	}

INLINE void pt_add(POINT *dst, const POINT *a, const POINT *b)
{	dst->x= a->x + b->x;	dst->y= a->x + a->y;	}

INLINE void pt_scale(POINT *dst, const POINT *a, int c)
{	dst->x= a->x*c;	dst->y= a->y*c;	}

INLINE BOOL pt_in_rect(const POINT *pt, const RECT *rc)
{
	return (pt->x < rc->l  || pt->y < rc->t ||
	        pt->x >= rc->r || pt->y >= rc->b) ? FALSE : TRUE;
}

// --- rect ---
INLINE void rc_set(RECT *rc, int l, int t, int r, int b)
{	rc->l=l;	rc->t=t;	rc->r=r;	rc->b=b;	}

INLINE void rc_set2(RECT *rc, int x, int y, int w, int h)
{	rc->l=x;	rc->t=y;	rc->r=x+w;	rc->b=y+h;	}

INLINE int rc_width(const RECT *rc)
{	return rc->r - rc->l;	}
INLINE int rc_height(const RECT *rc)
{	return rc->b - rc->t;	}

INLINE void rc_size(RECT *rc, int w, int h)
{	rc->r= rc->l+w;	rc->b= rc->t+h;	}

INLINE void rc_move(RECT *rc, int dx, int dy)
{	rc->l += dx;	rc->t += dy;	rc->r += dx;	rc->b += dy;	}
INLINE void rc_inflate(RECT *rc, int dw, int dh)
{	rc->l -= dw;	rc->t -= dh;	rc->r += dw;	rc->b += dh;	}
INLINE void rc_inflate2(RECT *rc, const RECT *dr)
{	rc->l += dr->l;	rc->t += dr->t;	rc->r += dr->r;	rc->b += dr->b;	}

// --- vector ---
INLINE void vec_set(VECTOR *a, FIXED x, FIXED y, FIXED z)
{	a->x= x;	a->y= y;	a->z= z;				}





#endif // GBA_GEOM
