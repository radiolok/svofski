// geom.c: 
// geometry implementation
//
// (Created: 2003-06-14, Modified: 2004-07-08, Cearn)

#include "geom.h"
#include "core.h"

// === CONSTANTS ======================================================
// === CLASSES ========================================================
// === PROTOTYPES =====================================================
// === MACROS =========================================================
// === INLINES=========================================================

// === FUNCTIONS ======================================================

// --- point ----------------------------------------------------------
// --- rect -----------------------------------------------------------

void rc_pos(RECT *rc, int x, int y)
{
	int w=rc->r-rc->l, h= rc->b-rc->t;
	rc_set2(rc, x, y, w, h);
}

void rc_norm(RECT *rc)
{
	int tmp;
	if(rc->r < rc->l)
		SWAP(rc->l, rc->r, tmp);
	if(rc->b < rc->t)
		SWAP(rc->t, rc->b, tmp);
}

// --- vector ---------------------------------------------------------
void vec_add(VECTOR *dst, const VECTOR *a, const VECTOR *b)
{
	dst->x = a->x + b->x;
	dst->y = a->y + b->y;
	dst->z = a->z + b->z;
}

void vec_sub(VECTOR *dst, const VECTOR *a, const VECTOR *b)
{
	dst->x = a->x - b->x;
	dst->y = a->y - b->y;
	dst->z = a->z - b->z;
}


void vec_scale(VECTOR *dst, const VECTOR *a, FIXED c)
{
	dst->x= (c*a->x)>>8;
	dst->y= (c*a->y)>>8;
	dst->z= (c*a->z)>>8;
}

FIXED vec_dot(const VECTOR *a, const VECTOR *b)
{	return (a->x*b->x + a->y*b->y + a->z*b->z)>>8;		}

void vec_cross(VECTOR *dst, const VECTOR *a, const VECTOR *b)
{
	dst->x= (a->y*b->z - a->z*b->y)>>8;
	dst->y= (a->z*b->x - a->x*b->z)>>8;
	dst->z= (a->x*b->y - a->y*b->x)>>8;
}
