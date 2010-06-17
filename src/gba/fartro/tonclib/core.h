// core.h: 
// basic gba functions
//
// (Created: 2004-04-28, Modified: 2004-07-05, Cearn)

#ifndef GBA_CORE
#define GBA_CORE

#include "types.h"
#include "regs.h"

// === CONFIGURATION ==================================================

#define CLD_FIXED24_8		// Use 24.8 fixed-point format

// === misc ===========================================================

// some bit operations
#define BIT(n)                  ( (1)<<n )
#define BIT_SET(word, flag)     ( word |=  (flag) )
#define BIT_CLEAR(word, flag)   ( word &= ~(flag) )
#define BIT_FLIP(word, flag)    ( word ^=  (flag) )
#define BIT_EQ(word, flag)      ( ((word)&(flag)) == (flag) )

// If you want your data in specific sections, add this 
// to your variables or functions.
// NOTE: a voice in the back of my head tells me these defines aren't 
// quite complete, but I can't remember what the full thing is :(
#define IN_EWRAM __attribute__ ((section(".ewram")))
#define IN_IWRAM __attribute__ ((section(".iwram")))
#define CODE_IN_IWRAM __attribute__ ((section(".iwram"),long_call))

// Add this to your program to make it multibootable
// Only required for dkAdv; dkARM uses linker specs to deal with this
#define MULTIBOOT int __gba_multiboot= 1;


// --- MATHEMATICS ----------------------------------------------------

// ---fixed point ---
#ifdef CLD_FIXED24_8

//typedef s32 FIXED			// inside types.h
#define FIX_SH 8			// 8bit fraction
#define FIX_SC 0x0100		// (1<<FIX_SH)

#define INT2FIX(n)  ((n)<<FIX_SH)
#define FIX2INT(n)  ((n)>>FIX_SH)
#define FIX_FRAC(n) ((n)&0x00ff)     // n&(FIX_SCALE-1)

#define FIX_SCF   256.0f 
#define FIX_SCF_  0.0004f

#define FIX2FLOAT(n)  ( (n)*FIX_SCF_ )
#define FLOAT2FIX(n)  ( FIXED((n)*FIX_SCF) ) 

#endif // CLD_FIXED


// --- boundary macros (you know you want them) ---

#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b)) 
#define MIN(a, b)  (((a) < (b)) ? (a) : (b)) 
#endif	// MAX

// absolute value
#ifndef ABS
#define ABS(x)       (((x) >= 0) ? (x) : (-(x)))
#endif	// ABS

// sign
#ifndef SGN
#define SGN(x)       (((x) >= 0) ? 1 : -1)
#endif	// SGN

// swaps a and b (t is a temporary variable!)
#ifndef SWAP
#define SWAP(a,b,t)	{ (t)=(a); (a)=(b); (b)=(t); }
#endif	// SWAP

// wraps num to stay in range [min,max]
#ifndef WRAP
#define WRAP(num, min, max)							\
{													\
	if((num)<(min))      (num) += (max)-(min)+1;	\
	else if((num)>(max)) (num) += (min)-(max)-1;	\
}

// truncates num to stay in range [min,max]
#define CLAMP(num, min, max)				\
{											\
	if((num)<(min))      (num)= (min);		\
	else if((num)>(max)) (num)= (max);		\
}

// reflects num at boundaries
#define REFLECT(num, min, max)						\
{													\
	if((num)<(min))      (num)= ((min)<<1)-(num);	\
	else if((num)>(max)) (num)= ((max)<<1)-(num);	\
}
#endif	// WRAP

// === CORE FUNCTIONS =================================================

void memset16(void *dest, u16 val, int nn);
void memcpy16(void *dest, const void* src, int nn);
void memrot16(void *mem, int first, int last);

// === DMA stuff ======================================================

// DMA transfer functionality

#define DMA_ON              0x80000000
#define DMA_IRQ             0x40000000
#define DMA_AT_NOW          0x00000000
#define DMA_AT_VBLANK       0x10000000
#define DMA_AT_HBLANK       0x20000000
#define DMA_AT_FIFO         0x30000000		// for sound ( DMA 1 & 2)
#define DMA_AT_REFRESH      0x30000000		// for video ( DMA 3)
#define DMA_16              0x00000000
#define DMA_32              0x04000000
#define DMA_REPEAT          0x02000000
#define DMA_SRC_INC         0x00000000
#define DMA_SRC_DEC         0x00800000
#define DMA_SRC_FIX         0x01000000
#define DMA_SRC_RESET       0x00600000
#define DMA_DST_INC         0x00000000
#define DMA_DST_DEC         0x00200000
#define DMA_DST_FIX         0x00400000
#define DMA_DST_RESET       0x00600000

#define DMA_32NOW           DMA_ON | DMA_AT_NOW |DMA_32
#define DMA_16NOW           DMA_ON | DMA_AT_NOW |DMA_16

typedef struct tagDMAINFO
{
  const void *src;
  void *dst;
  u32 cnt;
} DMAINFO;

#define dma_mem ((volatile DMAINFO*)0x040000b0)

// DMA transfer macro; looks like a function, but none of the 
//   penalties I luvs meh preprocessor :)
// NOTE: (2004-05-13)
//   I have switched the arguments to make it act more like memcpy
//   Only ch and mode are different. (Well, duh, memcpy doesn't have 
//   those)
// NOTE: apparently the real transfer doesn't start till 2 cycles 
//   after you set the DMAxCNT, so using this twice in a row may 
//   result in badness. (Haven't had any problems myself yet, though)
#define DMA_TRANSFER(dst, src, count, ch, mode)	\
{												\
	REG_DMA##ch##SAD = (u32)(src);				\
	REG_DMA##ch##DAD = (u32)(dst);				\
	REG_DMA##ch##CNT = (count) | (mode);		\
}

// separate enable and disable macros (redundant, but what the hell)
#define DMA_ENABLE(ch)	REG_DMA##ch##CNT |= DMA_ON
#define DMA_DISABLE(ch)	REG_DMA##ch##CNT &= ~DMA_ON


// --- dma prototypes

// Taken almost directly from pin8gba.h by Damian Yerrick
// Works exactly like the normal memcpy
// EXCEPT that it transfers per 32bit, not per 8 bit, and 
// count is the number of words (u32), not bytes (u8).
// Uses channel 3
// I could have used DMA_TRANSFER for this as well, but the jump back 
// to normal code ensures the proper rest time
void dma_memcpy(void *dst, const void *src, u16 count);

// === TIMERS =========================================================

// CPU frequency is 2^24 Hz
#define TM_BASE_FREQ   0x01000000

#define TM_FREQ_SYS    0x0000
#define TM_FREQ_1      0x0000
#define TM_FREQ_64     0x0001
#define TM_FREQ_256    0x0002
#define TM_FREQ_1024   0x0003
#define TM_CASCADE     0x0004
#define TM_IRQ         0x0040
#define TM_ON          0x0080


#endif // GBA_CORE

