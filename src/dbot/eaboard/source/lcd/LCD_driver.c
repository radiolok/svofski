/**
 Nokia 6100 (Philips) LCD Driver.

 This code is based on SparkFun example. I had to make quite a few modifications because
 the code didn't really work with Philips controller. 

 The original can be found at: 
 http://www.sparkfun.com/commerce/product_info.php?products_id=569

 */


//********************************************************************
//
//				LCD_driver.c: Interface for Nokia LCD
//
//********************************************************************

#include <stdlib.h>
#include <inttypes.h>

#include "LCD_driver.h"
#include "LPC210x.h"

#include "common.h"

#define R(x) (((x)>>8)&15)
#define G(x) (((x)>>4)&15)
#define B(x) (((x)>>0)&15)


void LCDSetPixel(int color, unsigned char x, unsigned char y);
void LCDClear(int color);
void LCDData(unsigned char data);
void LCDInit(void);

void LCDSend(uint16_t data);
void mdelay(uint16_t);

#if UBERSLOW
void udelay() {
    for(int i = 0; i < 16; i++) __asm__ volatile("nop");
}
#else
#define udelay()
#endif

///********************************************************************
//
//				Global Variables for LCD driver
//
//********************************************************************
//Usage: LCDClear(black);
//Inputs: char color: 8-bit color to be sent to the screen.
//Outputs: None
//Description: This function will clear the screen with "color" by writing the
//			   color to each location in the RAM of the LCD.
#if 0
void LCDClear(int color)
{
    LCDCommand(PASETP);
    LCDData(0);
    LCDData(131);

    LCDCommand(CASETP);
    LCDData(0);
    LCDData(131);

    LCDCommand(RAMWRP);

    uint16_t w1 = (R(color)<<4) | G(color);
    uint16_t w2 = (B(color)<<4) | R(color);
    uint16_t w3 = (G(color)<<4) | B(color);

	for(int i=0; i < (132*132)/2; i++) {
        LCDData(w1);
        LCDData(w2);
        LCDData(w3);
	}
}
#endif

// Description: Sends a 9 bit command over SPI 
void LCDSend(uint16_t data)
{
	GPIO_IOCLR = LCD_CS;      // enable chip, p0.20 goes low
    udelay();

    for (char j = 0; j < 9; j++) {
        if ((data & 0x100) == 0x100) {
            GPIO_IOSET = LCD_DIO;
        } else {
            GPIO_IOCLR = LCD_DIO;
        }

        GPIO_IOSET = LCD_SCK;   // send clock pulse
        udelay();
        GPIO_IOCLR = LCD_SCK;
        udelay();

        data <<= 1;
    }

    GPIO_IOSET = LCD_CS;    		// disable
    udelay();
}

void LCDCommand(uint8_t data)
{
    LCDSend(data);
}

void LCDData(uint8_t data)
{
    LCDSend(((uint16_t)data) | 0400);
}

void mdelay(uint16_t nops) {
    for (int j = 0; j < nops; j++) __asm__ volatile ("nop");
}

//Usage: LCDInit();
//Inputs: None
//Outputs: None
//Description:  Initializes the Philips LCD
void LCDInit(void)
{
	GPIO_IODIR |= (LCD_DIO | LCD_SCK | LCD_CS | LCD_RES);		//Assign LCD pins as Outputs	
	
    // reset display
    GPIO_IOCLR = (LCD_SCK | LCD_DIO);
    GPIO_IOSET = LCD_CS;
    mdelay(16);
    GPIO_IOCLR = LCD_RES;	
    mdelay(300);
    GPIO_IOSET = LCD_RES;
    mdelay(300);

	LCDCommand(SLEEPOUT);	//sleep out(PHILIPS)
    LCDCommand(BSTRON);		//Booset On(PHILIPS)
	LCDCommand(INVON);		// invert display mode(PHILIPS)
	LCDCommand(MADCTL);		//Memory Access Control(PHILIPS)
	LCDData(0xC8);

	LCDCommand(COLMOD);		//Set Color Mode(PHILIPS)
	LCDData(0x03);	
	
	LCDCommand(SETCON);		//Set Contrast(PHILIPS)
	LCDData(0x30);	
	   
	LCDCommand(NOPP);		// nop(PHILIPS)

	LCDCommand(DISPON);	// display on(PHILIPS)
}

//Usage: LCDSetPixel(white, 0, 0);
//Inputs: unsigned char color - desired color of the pixel
//		  unsigned char x - Page address of pixel to be colored
//		  unsigned char y - column address of pixel to be colored
//Outputs: None
//Description: Sets the starting page(row) and column (x & y) coordinates in ram,
//  		   then writes the colour to display memory.  The ending x & y are left
//  		   maxed out so one can continue sending colour data bytes to the 'open'
//  		   RAMWR command to fill further memory.  issuing any red command
//  		   finishes RAMWR.
void LCDSetPixel(int color, unsigned char x, unsigned char y) {
    LCDCommand(CASETP);   // column start/end ram
    LCDData(x);
    LCDData(ENDPAGE);

    LCDCommand(PASETP);   // page start/end ram
    LCDData(y);
    LCDData(ENDCOL);

    LCDCommand(RAMWRP);    // write
    
    LCDData((unsigned char)((color>>4)&0x00FF));
    LCDData((unsigned char)(((color&0x0F)<<4)|0x00));
}

uint16_t LCDSetClipRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    int ylast = y + h;
    if (ylast > 132) {
        ylast = 132;
    }

    int xlast = x + w;
    if (xlast > 132) {
        xlast = 132;
    }

    LCDCommand(CASETP);
    LCDData(x);
    LCDData(xlast);
    LCDCommand(PASETP);
    LCDData(y);
    LCDData(ylast);

    LCDCommand(RAMWRP);    // write

    return (ylast-y+1)*(xlast-x+1);
}

void LCDFillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
    uint16_t count = LCDSetClipRect(x, y, w, h) / 2 + 1;
    uint16_t w1 = (R(color)<<4) | G(color);
    uint16_t w2 = (B(color)<<4) | R(color);
    uint16_t w3 = (G(color)<<4) | B(color);

    for (; count--; ) {
        LCDData(w1);
        LCDData(w2);
        LCDData(w3);
    }
}

void LCDClear(int color) {
    LCDFillRect(1, 1, 131, 131, color);
}
