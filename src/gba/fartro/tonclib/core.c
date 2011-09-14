// core.c:
// core GBA functions
//
// (Created: 2004-04-28, Modified: 2004-04-28, Cearn)

#include "types.h"
#include "regs.h"
#include "core.h"

// === 16bit memory routines ==========================================
//
// HEAVY optimisations possible methinks
void memset16(void *dest, u16 val, int nn)
{
	int ii;
	u16* _dest= (u16*)dest;
	for(ii=0; ii<nn; ii++)
		_dest[ii]= val;
}

void memcpy16(void *dest, const void* src, int nn)
{
	int ii;
	u16 *_src= (u16*)src, *_dest= (u16*)dest;
	for(ii=0; ii<nn; ii++)
		_dest[ii]= _src[ii];
}

// last>first: rotate right | dir= -1
// last<first: rotate left  | dir= +1
void memrot16(void *mem, int first, int last)
{
	if(first == last)
		return;

	int ii, dir =( last>first ? -1 : 1);
	u16* _mem= (u16*)mem, tmp;
	// store [last], since it'll be overwritten and has nowhere to go
	tmp= _mem[last];
	// copy from [last] to [first] !!
	for(ii=last; ii!=first; ii += dir)
		_mem[ii]= _mem[ii+dir];
	// finish up rotate
	_mem[first]= tmp;
}

// === DMA ============================================================

// see core.h for description
void dma_memcpy(void *dst, const void *src, u16 count)
{
	dma_mem[3].cnt = 0;	// shut off any previous transfer
	dma_mem[3].src = src;
	dma_mem[3].dst = dst;
	dma_mem[3].cnt = count | DMA_32NOW;
}

// === TIMERS =========================================================

