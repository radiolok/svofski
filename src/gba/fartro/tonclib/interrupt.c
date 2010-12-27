// interrupt.c: 
// interrupt implementation file
//
// (Created: 2003-11, Modified: 2004-05-21, Cearn)

#include "types.h"
#include "regs.h"
#include "interrupt.h"


// === CLASSES ========================================================

// used for int_enable_ex and int_disable_ex
typedef struct tagINT_SENDER
{
	u16 reg_ofs, flag;
}	INT_SENDER;

// === GLOBALS ========================================================

fnptr IntrTable[] = 
{
	int_dummy,		// INT_VBLANK
	int_dummy,		// INT_HBLANK
	int_dummy,		// INT_VCOUNT
	int_dummy,		// INT_TM0
	int_dummy,		// INT_TM1
	int_dummy,		// INT_TM2
	int_dummy,		// INT_TM3
	int_dummy,		// INT_COM
	int_dummy,		// INT_DMA0
	int_dummy,		// INT_DMA1
	int_dummy,		// INT_DMA2
	int_dummy,		// INT_DMA3
	int_dummy,		// INT_KEYS
	int_dummy,		// INT_CART
};

// yeah, yeah, I really should use the registers and defines
// I have else where.
// NOTE: haven't really tested this very much; if inaccurate, 
// plz tell me
static const INT_SENDER _int_senders[] = 
{
	{ 0x0004, 0x0008 },		// REG_DISPSTAT, VID_IRQ_VB
	{ 0x0004, 0x0010 },		// REG_DISPSTAT, VID_IRQ_VH
	{ 0x0004, 0x0020 },		// REG_DISPSTAT, VID_IRQ_VC
	{ 0x0102, 0x0040 },		// REG_TM0CNT, TM_IRQ
	{ 0x0106, 0x0040 },		// REG_TM1CNT, TM_IRQ
	{ 0x0108, 0x0040 },		// REG_TM2CNT, TM_IRQ
	{ 0x0102, 0x0040 },		// REG_TM3CNT, TM_IRQ
	{ 0x0128, 0x4000 },		// REG_SCCNT_L{14}			// unsure
	{ 0x00ba, 0x4000 },		// REG_DMA0CNT_H, DMA_IRQ>>16
	{ 0x00c6, 0x4000 },		// REG_DMA1CNT_H, DMA_IRQ>>16
	{ 0x00d2, 0x4000 },		// REG_DMA2CNT_H, DMA_IRQ>>16
	{ 0x00de, 0x4000 },		// REG_DMA3CNT_H, DMA_IRQ>>16
	{ 0x0132, 0x4000 },		// REG_P1CNT, KEY_CNT_IRQ
	{ 0x0000, 0x0000 },		// none
};
// === PROTOTYPES =====================================================

// === FUNCTIONS ======================================================

// IntrMain is an asm function in single_ints.s
void int_init()
{
	REG_INTMAIN = IntrMain;
}

void int_enable_ex(enum eIntrIndex eii, fnptr isr)
{
	REG_IME=0;

	const INT_SENDER *is= &_int_senders[eii];
	*(u16*)(REG_BASE+is->reg_ofs) |= is->flag;
	if(isr != NULL)
		IntrTable[eii]= isr;

	REG_IE |= 1 << eii;
	REG_IME= 1;
}

void int_disable_ex(enum eIntrIndex eii)
{
	u16 tmp= REG_IME;
	REG_IME= 0;

	const INT_SENDER *is= &_int_senders[eii];
	*(u16*)(REG_BASE+is->reg_ofs) &= ~is->flag;

	REG_IE &= ~(1 << eii);
	REG_IME= tmp;
}
 

// --- dummy isrs ---
// (no need to set REG_IF or REG_IFBIOS bits, 
//  single_ints.s' switchboard does that for us)
void int_dummy() {}


