/*	LCD-color.h
*
*	Class for controlling Spark Fun's 130x130 color LCD.
*	The LCD requires 9-bit serial communication, which this
*	code does via the SPI feature.
*
*	Revisions:
*		07-13-06	included in LCDSample project
*		07-10-06	version 1 in master library
*
*	Written by Cathy Saxton
*	robotics@idleloop.com
*/

#pragma once

#include <stdio.h>
#include <inttypes.h>
#include "common.h"
#include "InOut.h"
#include "Fonts.h"

/*
coordinate system
	upper left corner is (0,0)
	positive x is to the right
	positive y is down
variable names
	dx = "delta x" (or "difference x") which is width
	dy = "delta y" which is height
	clr = color
*/

const uint16_t dxLCDScreen = 132;	// number of horizontal pixels on LCD screen
const uint16_t dyLCDScreen = 130;	// number of vertical pixels on LCD screen

/* CoLoRs */
enum
{
	/* define a special value to be treated as transparent when used as
	   the background color (when outputting text); technically all byte
	   values correspond to an actual color, but we'll override this
	   very dark green for that purpose */
	clrTransparent = 0x04,	// 000 001 00

	clrBlack	= 0x00,
	clrWhite	= 0xff,

	clrRed		= 0xe0,	// 111 000 00 (ff0000)
	clrGreen	= 0x1c,	// 000 111 00 (00ff00)
	clrBlue		= 0x03,	// 000 000 11 (0000ff)

	clrMedRed	= 0x80,	// 100 000 00 (880000)
	clrMedGreen	= 0x10,	// 000 100 00 (008800)
	clrMedBlue	= 0x02,	// 000 000 10 (000099)

	clrDkRed	= 0x40,	// 010 000 00 (440000)
	clrDkGreen	= 0x08,	// 000 010 00 (004400)
	clrDkBlue	= 0x01,	// 000 000 01 (000066)

	clrYellow	= 0xfc,	// 111 111 00 (ffff00)
	clrOrange	= 0xf0,	// 111 100 00 (ff8800)
	clrBrown	= 0x6c,	// 011 011 00 (666600)

	clrMagenta	= 0xe3,	// 111 000 11 (ff00ff)
	clrPurple	= 0x83,	// 100 000 11 (8800ff)

	clrCyan		= 0x1f,	// 000 111 11 (00ffff)
	clrTeal		= 0x17,	// 000 101 11 (00aaff)
	clrDkTeal	= 0x12,	// 000 100 10 (008899)

	clrLtRed	= 0xf2,
	clrLtOrange	= 0xf6,
	clrLtYellow	= 0xfe,
	clrLyGreen	= 0x9e,
	clrLtCyan	= 0x7f,
	clrLtBlue	= 0x6f,
	clrLtMagenta = 0xf3,
};

class LCD
{
public:
	LCD(OUT *poutE, OUT *poutMOSI, OUT *poutSCK, OUT *poutReset,
		const FONT *pfont, uint8_t clrBack, uint8_t clrFore);

    inline uint16_t getWidth() const  { return dxLCDScreen; }
    inline uint16_t getHeight() const { return dyLCDScreen; }
    
	/*
	*	text stuff
	*/

	/* prints given string on the specified line; unused portion of
	   line will be padded with background color by default; use a
	   1-based value for a specific line, or 0 for the current line;
	   line is updated after outputting text */
	void Print(const char *psz, uint8_t line, bool fPad = TRUE);

	/* sets current text output location to the specified position
	   (ich, 0-based) on the specified line; note that for proportional
	   fonts, horizontal position is an approximation (based on the
	   width of a space); using line=0 will leave line unchanged */
	void SetPos(uint8_t ich, uint8_t line);
	/* writes a character at the current location, updates current location */
	void WriteCh(char ch);

	/* set properties for rendering text */
	void SetFont(const FONT *pfont)   { if (pfont != NULL) m_pfont = pfont; }
	inline void SetBackClr(uint8_t clr) { m_clrBack = clr; }
	inline void SetForeClr(uint8_t clr) { m_clrFore = clr; }

	/*
	*	graphic stuff
	*/

	/* set the color of given pixel; note that pixels off screen will be ignored */
	void ColorPixel(uint8_t x, uint8_t y, uint8_t clr) const;

	/* draw a solid/filled rectangle with its upper left corner at (x, y),
	   with the given width, height, and color */
	void FillRect(uint8_t x, uint8_t y, uint8_t dx, uint8_t dy, uint8_t clr) const;
	/* outline a rectangle with its upper left corner at (x, y),
	   with the given width, height, and color */
	void FrameRect(uint8_t x, uint8_t y, uint8_t dx, uint8_t dy, uint8_t clr) const;

	/* draw a horizontal line across the screen at position y
	   (0-based from the top) */
	inline void DrawHLine(uint8_t y, uint8_t clr) const
		{ FillRect(0, y, dxLCDScreen, 1, clr); }
	/* draw a vertical line across the screen at position y
	   (0-based from the top) */
	inline void DrawVLine(uint8_t x, uint8_t clr) const
		{ FillRect(x, 0, 1, dyLCDScreen, clr); }

	/* draw a line from (x1, y1) to (x2, y2) */
	void DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t clr) const;

	/* draw the outline of a circle */
	void FrameCircle(uint8_t xCenter, uint8_t yCenter, uint8_t r, uint8_t clr) const;
	/* draw a solid/filled circle */
	void FillCircle(uint8_t xCenter, uint8_t yCenter, uint8_t r, uint8_t clr) const;

	/* draw the specified bitmap (located in flash memory) at the given
	   coordinates (upper left corner); the first two bytes of the memory
	   specify the width and height, with remaining bytes as 8-bit color
	   values; will not draw anything if not enough room for full image */
	void ShowBitmap(const uint8_t *pab, uint8_t x, uint8_t y) const;

	/*
	*	LCD setup
	*/

	/* sets "volume" for the LCD, which is essentially a brightness
	   level for the LED backlight;
	   ratio: 0-7, "resistance ratio of built-in voltage regulating resistor" 
		  this is a coarse setting
	   volume: 0-63, "electronic volume value"
		  this will fine-tune the brightness
	   technique -- find a ratio value that works well, then tune volume */
	void SetVolume(uint8_t ratio, uint8_t volume) const;
	/* increment/decrement volume; note that this appears to change
	   ratio when volumn wraps, resulting in a cycle of 512 values */
	void IncVolumn() const;
	void DecVolumn() const;

private:
	void Init() const;
	void WriteCommand(uint8_t lct, uint8_t ch) const;
    void WriteCommand(uint16_t) const;
	uint16_t CpxSetAndValidateLCDRect(uint8_t x, uint8_t y, uint8_t dx, uint8_t dy) const;
    void mdelay(uint16_t) const;

	const FONT *m_pfont;
	uint8_t m_clrBack, m_clrFore;
	uint8_t m_x, m_y;		// current position

	/* static variables: per class, not instance */
	static bool s_fInit;
	static OUT *s_poutE;
	static OUT *s_poutSCK;
	static OUT *s_poutMOSI;
	static OUT *s_poutReset;
};
