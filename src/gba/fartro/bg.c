#define BG_C

#include "gba.h"
#include "bg.h"


#define GBA_TYPES

#include "luts.h"

void EnableBackground(Bg* bg)
{
	u16 temp;

	bg->tileData = (u16*)CharBaseBlock(bg->charBaseBlock);
	bg->mapData = (u16*)ScreenBaseBlock(bg->screenBaseBlock);
	temp = bg->size | (bg->charBaseBlock<<CHAR_SHIFT) | (bg->screenBaseBlock<<SCREEN_SHIFT)
		| bg->colorMode | bg->mosaic | bg->wraparound;

	switch(bg->number)
	{
	case 0:
		{
			REG_BG0CNT = temp;
			REG_DISPCNT |= BG0_ENABLE;
		}break;
	case 1:
		{
			REG_BG1CNT = temp;
			REG_DISPCNT |= BG1_ENABLE;
		}break;
	case 2:
		{
			REG_BG2CNT = temp;
			REG_DISPCNT |= BG2_ENABLE;
		}break;
	case 3:
		{
			REG_BG3CNT = temp;
			REG_DISPCNT |= BG3_ENABLE;
		}break;

	default:break;

	}
}

void UpdateBackground(Bg* bg)
{
	switch(bg->number)
	{
	case 0:
		REG_BG0HOFS = bg->x_scroll;
		REG_BG0VOFS = bg->y_scroll;
		break;
	case 1:
		REG_BG1HOFS = bg->x_scroll;
		REG_BG1VOFS = bg->y_scroll;
		break;
	case 2:
		if(!(REG_DISPCNT & MODE_0))//it is a rot background
		{
			REG_BG2X = bg->DX;
			REG_BG2Y = bg->DY;

			//REG_BG2PA = REG_BG2PD = 56;
			//REG_BG2PB = REG_BG2PC = 0;
			REG_BG2PA = bg->PA;
			REG_BG2PB = bg->PB;
			REG_BG2PC = bg->PC;
			REG_BG2PD = bg->PD;
		}
		else  //it is a text background
		{
			REG_BG2HOFS = bg->x_scroll;
			REG_BG2VOFS = bg->y_scroll;
		}
		break;
	case 3:
		if(!(REG_DISPCNT & MODE_0))//it is a rot background
		{
			REG_BG3X = bg->DX;
			REG_BG3Y = bg->DY;

			REG_BG3PA = bg->PA;
			REG_BG3PB = bg->PB;
			REG_BG3PC = bg->PC;
			REG_BG3PD = bg->PD;
		}
		else //it is a text background
		{
			REG_BG3HOFS = bg->x_scroll;
			REG_BG3VOFS = bg->y_scroll;
		}
		break;
	default: break;
	}
}

void RotateBackground(Bg* bg, int angle,int center_x, int center_y, FIXED zoom)
{

	center_y = (center_y * zoom)>>8;
	center_x = (center_x * zoom)>>8;


	bg->PA = (lut_cos(angle)*zoom) >> 8;  //cos&sin are LUTs that are .8 fixed numbers
	bg->PB = -(lut_sin(angle)*zoom) >> 8;  //zoom is also fixed
	bg->PC = (lut_sin(angle)*zoom) >> 8;
	bg->PD = (lut_cos(angle)*zoom) >> 8;

	bg->DX = ((bg->x_scroll<<8) - (center_y * bg->PA + center_x*bg->PB));
	bg->DY = ((bg->y_scroll<<8) - (center_y * bg->PC + center_x*bg->PD));
}
