#ifndef BG_H
#define BG_H


///BGCNT defines ///
#define BG_MOSAIC_ENABLE		0x40
#define BG_COLOR_256			0x80
#define BG_COLOR_16			0x0

#define CharBaseBlock(n)		(((n)*0x4000)+0x6000000)
#define ScreenBaseBlock(n)		(((n)*0x800)+0x6000000)

#define CHAR_SHIFT			2
#define SCREEN_SHIFT			8
#define TEXTBG_SIZE_256x256		0x0
#define TEXTBG_SIZE_256x512		0x8000
#define TEXTBG_SIZE_512x256		0x4000
#define TEXTBG_SIZE_512x512		0xC000

#define ROTBG_SIZE_128x128		0x0
#define ROTBG_SIZE_256x256		0x4000
#define ROTBG_SIZE_512x512		0x8000
#define ROTBG_SIZE_1024x1024		0xC000

#define WRAPAROUND              	0x1

typedef struct Bg
{
	u16* tileData;
	u16* mapData;
	u8 mosaic;
	u8 colorMode;
	u8 number;
	u16 size;
	u8 charBaseBlock;
	u8 screenBaseBlock;
	u8 wraparound;
	s16 x_scroll,y_scroll;
	s32 DX,DY;
	s16 PA,PB,PC,PD;
}Bg;

#ifndef BG_C
extern void EnableBackground(Bg* bg);
extern void RotateBackground(Bg* bg, int angle, int center_x, int center_y, FIXED zoom);
extern void UpdateBackground(Bg* bg);
#endif
#endif
