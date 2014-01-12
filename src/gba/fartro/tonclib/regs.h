// regs.h
// gba registers
// originally by eloist
// Modified by Cearn
//
// NOTE: the REG_BGxy registers for affine backgrounds
// should be _signed_ (vs16 / vs32), not unsigned (vu16 / vu32)
//
// (Created: 2003-05-05, Modified: 2004-03-20, Cearn)

#ifndef GBA_REGS
#define GBA_REGS

#include "types.h"

// memory sectors
#define MEM_EWRAM   0x02000000
#define MEM_IWRAM   0x03000000
#define MEM_IO      0x04000000
#define MEM_PAL     0x05000000
#define MEM_VRAM    0x06000000
#define MEM_OAM     0x07000000
#define MEM_ROM     0x08000000
#define MEM_SRAM    0x0e000000

#define REG_BASE    MEM_IO

// 0300:7ff[y] is mirrored at 03ff:fff[y], which is why this works out:
#define REG_IFBIOS        *(vu16*)(REG_BASE-0x0008)	// -> 0300:7ff8
#define REG_RESET_DST     *(vu16*)(REG_BASE-0x0006)	// -> 0300:7ffa
#define REG_INTMAIN      *(fnptr*)(REG_BASE-0x0004)	// -> 0300:7ffc

// === VIDEO REGISTERS ===
#define REG_DISPCNT       *(vu32*)(REG_BASE+0x0000)	// display control
#define REG_DISPCNT_L     *(vu16*)(REG_BASE+0x0000)
#define REG_DISPCNT_H     *(vu16*)(REG_BASE+0x0002)
#define REG_DISPSTAT      *(vu16*)(REG_BASE+0x0004)	// display interupt status
#define REG_VCOUNT        *(vu16*)(REG_BASE+0x0006)	// vertical count
// --- background ---
#define REG_BG0CNT        *(vu16*)(REG_BASE+0x0008)	// bg 0-3 control
#define REG_BG1CNT        *(vu16*)(REG_BASE+0x000A)
#define REG_BG2CNT        *(vu16*)(REG_BASE+0x000C)
#define REG_BG3CNT        *(vu16*)(REG_BASE+0x000E)
#define REG_BG0HOFS       *(vu16*)(REG_BASE+0x0010)	// bg 0-3 origins
#define REG_BG0VOFS       *(vu16*)(REG_BASE+0x0012)
#define REG_BG1HOFS       *(vu16*)(REG_BASE+0x0014)
#define REG_BG1VOFS       *(vu16*)(REG_BASE+0x0016)
#define REG_BG2HOFS       *(vu16*)(REG_BASE+0x0018)
#define REG_BG2VOFS       *(vu16*)(REG_BASE+0x001A)
#define REG_BG3HOFS       *(vu16*)(REG_BASE+0x001C)
#define REG_BG3VOFS       *(vu16*)(REG_BASE+0x001E)
#define REG_BG2PA         *(vs16*)(REG_BASE+0x0020)	// bg 2 affine matrix..
#define REG_BG2PB         *(vs16*)(REG_BASE+0x0022)	// | pa  pb |
#define REG_BG2PC         *(vs16*)(REG_BASE+0x0024)	// | pc  pd |
#define REG_BG2PD         *(vs16*)(REG_BASE+0x0026)
#define REG_BG2X          *(vs32*)(REG_BASE+0x0028)	// bg affine org
#define REG_BG2X_L        *(vu16*)(REG_BASE+0x0028)
#define REG_BG2X_H        *(vs16*)(REG_BASE+0x002A)
#define REG_BG2Y          *(vs32*)(REG_BASE+0x002C)
#define REG_BG2Y_L        *(vu16*)(REG_BASE+0x002C)
#define REG_BG2Y_H        *(vs16*)(REG_BASE+0x002E)
#define REG_BG3PA         *(vs16*)(REG_BASE+0x0030)	// bg 3 affine matrix..
#define REG_BG3PB         *(vs16*)(REG_BASE+0x0032)	// | pa  pb |
#define REG_BG3PC         *(vs16*)(REG_BASE+0x0034)	// | pc  pd |
#define REG_BG3PD         *(vs16*)(REG_BASE+0x0036)
#define REG_BG3X          *(vs32*)(REG_BASE+0x0038)	// bg 3 affine org
#define REG_BG3X_L        *(vu16*)(REG_BASE+0x0038)
#define REG_BG3X_H        *(vs16*)(REG_BASE+0x003A)
#define REG_BG3Y          *(vs32*)(REG_BASE+0x003C)
#define REG_BG3Y_L        *(vu16*)(REG_BASE+0x003C)
#define REG_BG3Y_H        *(vs16*)(REG_BASE+0x003E)
/// --- windowing ---
#define REG_WIN0H         *(vu16*)(REG_BASE+0x0040) // win0 left, right
#define REG_WIN1H         *(vu16*)(REG_BASE+0x0042) // ditto win1
#define REG_WIN0V         *(vu16*)(REG_BASE+0x0044) // win0 top, bottom
#define REG_WIN1V         *(vu16*)(REG_BASE+0x0046) // ditto win1
#define REG_WININ         *(vu16*)(REG_BASE+0x0048)	// win0, win1 control
#define REG_WINOUT        *(vu16*)(REG_BASE+0x004A)	// winOut, winObj control
// --- effects ---
#define REG_MOSAIC        *(vu32*)(REG_BASE+0x004C)	// mosaic control
#define REG_MOSAIC_L      *(vu32*)(REG_BASE+0x004C)
#define REG_MOSAIC_H      *(vu32*)(REG_BASE+0x004E)
#define REG_BLDMOD        *(vu16*)(REG_BASE+0x0050)	// alpha control
#define REG_COLEV         *(vu16*)(REG_BASE+0x0052)	// fade level
#define REG_COLEY         *(vu16*)(REG_BASE+0x0054)	// blend levels
// === SOUND REGISTERS ===
#define REG_SG10          *(vu32*)(REG_BASE+0x0060)
#define REG_SG10_L        *(vu16*)(REG_BASE+0x0060)
#define REG_SG10_H        *(vu16*)(REG_BASE+0x0062)
#define REG_SG11          *(vu16*)(REG_BASE+0x0064)
#define REG_SG20          *(vu16*)(REG_BASE+0x0068)
#define REG_SG21          *(vu16*)(REG_BASE+0x006C)
#define REG_SG30          *(vu32*)(REG_BASE+0x0070)
#define REG_SG30_L        *(vu16*)(REG_BASE+0x0070)
#define REG_SG30_H        *(vu16*)(REG_BASE+0x0072)
#define REG_SG31          *(vu16*)(REG_BASE+0x0074)
#define REG_SG40          *(vu16*)(REG_BASE+0x0078)
#define REG_SG41          *(vu16*)(REG_BASE+0x007C)
#define REG_SGCNT0        *(vu32*)(REG_BASE+0x0080)
#define REG_SGCNT0_L      *(vu16*)(REG_BASE+0x0080)
#define REG_SGCNT0_H      *(vu16*)(REG_BASE+0x0082)
#define REG_SGCNT1        *(vu16*)(REG_BASE+0x0084)
#define REG_SGBIAS        *(vu16*)(REG_BASE+0x0088)
#define REG_SGWR0         *(vu32*)(REG_BASE+0x0090)
#define REG_SGWR0_L       *(vu16*)(REG_BASE+0x0090)
#define REG_SGWR0_H       *(vu16*)(REG_BASE+0x0092)
#define REG_SGWR1         *(vu32*)(REG_BASE+0x0094)
#define REG_SGWR1_L       *(vu16*)(REG_BASE+0x0094)
#define REG_SGWR1_H       *(vu16*)(REG_BASE+0x0096)
#define REG_SGWR2         *(vu32*)(REG_BASE+0x0098)
#define REG_SGWR2_L       *(vu16*)(REG_BASE+0x0098)
#define REG_SGWR2_H       *(vu16*)(REG_BASE+0x009A)
#define REG_SGWR3         *(vu32*)(REG_BASE+0x009C)
#define REG_SGWR3_L       *(vu16*)(REG_BASE+0x009C)
#define REG_SGWR3_H       *(vu16*)(REG_BASE+0x009E)
#define REG_SGFIFOA       ((vu32*)(REG_BASE+0x00A0)
#define REG_SGFIFOA_L     *(vu16*)(REG_BASE+0x00A0)
#define REG_SGFIFOA_H     *(vu16*)(REG_BASE+0x00A2)
#define REG_SGFIFOB       ((vu32*)(REG_BASE+0x00A4)
#define REG_SGFIFOB_L     *(vu16*)(REG_BASE+0x00A4)
#define REG_SGFIFOB_H     *(vu16*)(REG_BASE+0x00A6)
// === DMA REGISTERS ===
#define REG_DMA0SAD       *(vu32*)(REG_BASE+0x00B0) // source
#define REG_DMA0SAD_L     *(vu16*)(REG_BASE+0x00B0)
#define REG_DMA0SAD_H     *(vu16*)(REG_BASE+0x00B2)
#define REG_DMA0DAD       *(vu32*)(REG_BASE+0x00B4) // destination
#define REG_DMA0DAD_L     *(vu16*)(REG_BASE+0x00B4)
#define REG_DMA0DAD_H     *(vu16*)(REG_BASE+0x00B6)
#define REG_DMA0CNT       *(vu32*)(REG_BASE+0x00B8) // control
#define REG_DMA0CNT_L     *(vu16*)(REG_BASE+0x00B8) // count
#define REG_DMA0CNT_H     *(vu16*)(REG_BASE+0x00BA) // flags
#define REG_DMA1SAD       *(vu32*)(REG_BASE+0x00BC)
#define REG_DMA1SAD_L     *(vu16*)(REG_BASE+0x00BC)
#define REG_DMA1SAD_H     *(vu16*)(REG_BASE+0x00BE)
#define REG_DMA1DAD       *(vu32*)(REG_BASE+0x00C0)
#define REG_DMA1DAD_L     *(vu16*)(REG_BASE+0x00C0)
#define REG_DMA1DAD_H     *(vu16*)(REG_BASE+0x00C2)
#define REG_DMA1CNT       *(vu32*)(REG_BASE+0x00C4)
#define REG_DMA1CNT_L     *(vu16*)(REG_BASE+0x00C4)
#define REG_DMA1CNT_H     *(vu16*)(REG_BASE+0x00C6)
#define REG_DMA2SAD       *(vu32*)(REG_BASE+0x00C8)
#define REG_DMA2SAD_L     *(vu16*)(REG_BASE+0x00C8)
#define REG_DMA2SAD_H     *(vu16*)(REG_BASE+0x00CA)
#define REG_DMA2DAD       *(vu32*)(REG_BASE+0x00CC)
#define REG_DMA2DAD_L     *(vu16*)(REG_BASE+0x00CC)
#define REG_DMA2DAD_H     *(vu16*)(REG_BASE+0x00CE)
#define REG_DMA2CNT       *(vu32*)(REG_BASE+0x00D0)
#define REG_DMA2CNT_L     *(vu16*)(REG_BASE+0x00D0)
#define REG_DMA2CNT_H     *(vu16*)(REG_BASE+0x00D2)
#define REG_DMA3SAD       *(vu32*)(REG_BASE+0x00D4)
#define REG_DMA3SAD_L     *(vu16*)(REG_BASE+0x00D4)
#define REG_DMA3SAD_H     *(vu16*)(REG_BASE+0x00D6)
#define REG_DMA3DAD       *(vu32*)(REG_BASE+0x00D8)
#define REG_DMA3DAD_L     *(vu16*)(REG_BASE+0x00D8)
#define REG_DMA3DAD_H     *(vu16*)(REG_BASE+0x00DA)
#define REG_DMA3CNT       *(vu32*)(REG_BASE+0x00DC)
#define REG_DMA3CNT_L     *(vu16*)(REG_BASE+0x00DC)
#define REG_DMA3CNT_H     *(vu16*)(REG_BASE+0x00DE)
// === TIMER REGISTERS ===
#define REG_TM0D          *(vu16*)(REG_BASE+0x0100)	// timers; data and control
#define REG_TM0CNT        *(vu16*)(REG_BASE+0x0102)
#define REG_TM1D          *(vu16*)(REG_BASE+0x0104)
#define REG_TM1CNT        *(vu16*)(REG_BASE+0x0106)
#define REG_TM2D          *(vu16*)(REG_BASE+0x0108)
#define REG_TM2CNT        *(vu16*)(REG_BASE+0x010A)
#define REG_TM3D          *(vu16*)(REG_BASE+0x010C)
#define REG_TM3CNT        *(vu16*)(REG_BASE+0x010E)
// === SERIAL COMMUNICATION ===
#define REG_SCD0          *(vu16*)(REG_BASE+0x0120)
#define REG_SCD1          *(vu16*)(REG_BASE+0x0122)
#define REG_SCD2          *(vu16*)(REG_BASE+0x0124)
#define REG_SCD3          *(vu16*)(REG_BASE+0x0126)
#define REG_SCCNT         *(vu32*)(REG_BASE+0x0128)
#define REG_SCCNT_L       *(vu16*)(REG_BASE+0x0128)
#define REG_SCCNT_H       *(vu16*)(REG_BASE+0x012A)
// === KEYPAD ===
#define REG_P1            *(vu16*)(REG_BASE+0x0130)
#define REG_P1CNT         *(vu16*)(REG_BASE+0x0132)
// === ===
#define REG_R             *(vu16*)(REG_BASE+0x0134)
#define REG_HS_CTRL       *(vu16*)(REG_BASE+0x0140)
#define REG_JOYRE         *(vu32*)(REG_BASE+0x0150)
#define REG_JOYRE_L       *(vu16*)(REG_BASE+0x0150)
#define REG_JOYRE_H       *(vu16*)(REG_BASE+0x0152)
#define REG_JOYTR         *(vu32*)(REG_BASE+0x0154)
#define REG_JOYTR_L       *(vu16*)(REG_BASE+0x0154)
#define REG_JOYTR_H       *(vu16*)(REG_BASE+0x0156)
#define REG_JSTAT         *(vu32*)(REG_BASE+0x0158)
#define REG_JSTAT_L       *(vu16*)(REG_BASE+0x0158)
#define REG_JSTAT_H       *(vu16*)(REG_BASE+0x015A)
// === INTERRUPT REGISTERS II ===
#define REG_IE            *(vu16*)(REG_BASE+0x0200)
#define REG_IF            *(vu16*)(REG_BASE+0x0202)
#define REG_WSCNT         *(vu16*)(REG_BASE+0x0204)
#define REG_IME           *(vu16*)(REG_BASE+0x0208)
#define REG_PAUSE         *(vu16*)(REG_BASE+0x0300)

#endif // GBA_REGS



