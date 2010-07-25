#ifndef _LCD_DRIVER_H
#define _LCD_DRIVER_H

#include <inttypes.h>
//********************************************************************
//
//				General Function Definitions
//
//********************************************************************
void LCDCommand(unsigned char data);
void LCDData(unsigned char data);
void LCDInit(void);
void LCDClear(int color);
void LCDSetPixel(int color, unsigned char x, unsigned char y);

void LCDFillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);


//*******************************************************
//					GPIO Definitions
//*******************************************************
#define LCD_RES (1<<25)// LCD_RST#
#define LCD_CS	(1<<24)// LCD_CS#
#define LCD_DIO	(1<<6) // MOSI
#define LCD_SCK (1<<4) // CLK

//********************************************************************
//
//					LCD Dimension Definitions
//
//********************************************************************
#define ROW_LENGTH	132
#define COL_HEIGHT	132
#define ENDPAGE     132
#define ENDCOL      132

//********************************************************************
//
//			PHILIPS Controller Definitions
//
//********************************************************************
//LCD Commands
#define	NOPP		0x00
#define	BSTRON		0x03
#define SLEEPIN     0x10
#define	SLEEPOUT	0x11
#define	NORON		0x13
#define	INVOFF		0x20
#define INVON      	0x21
#define	SETCON		0x25
#define DISPOFF     0x28
#define DISPON      0x29
#define CASETP      0x2A
#define PASETP      0x2B
#define RAMWRP      0x2C
#define RGBSET	    0x2D
#define	MADCTL		0x36
#define	COLMOD		0x3A
#define DISCTR      0xB9
#define	EC			0xC0

//*******************************************************
//				12-Bit Color Definitions
//*******************************************************
#define WHITE	0xFFF
#define BLACK	0x000
#define RED		0xF00
#define	GREEN	0x0F0
#define BLUE	0x00F
#define CYAN	0x0FF
#define MAGENTA	0xF0F
#define YELLOW	0xFF0
#define BROWN	0xB22
#define ORANGE	0xFA0
#define PINK	0xF6A

#endif

