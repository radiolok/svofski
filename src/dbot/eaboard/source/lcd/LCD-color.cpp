/*	LCD-color.cpp
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

/* ARM LPC210x port and Philips controller code 
 * by Viacheslav Slavinsky, 2010
 *
 * Both types of controllers should work. Define
 * PHILIPS or EPSON somewhere and it shall pick
 * up from there.
 */

#include <string.h>
#include <inttypes.h>
#include "LCD-color.h"
#include "Fonts.h"
#include "common.h"

/* horiz: 0-129 (inclusive); vert: 2-131 */
/* Hungarian naming: xl and yl are x and y coordinates on LCD
   (handling offset for visible pixels) */
static const uint16_t xlMin = 0;
static const uint16_t ylMin = 2;
static const uint16_t xlMax = xlMin + dxLCDScreen;
static const uint16_t ylMax = ylMin + dyLCDScreen;
static const uint16_t xlMost = xlMin + dxLCDScreen - 1;
static const uint16_t ylMost = ylMin + dyLCDScreen - 1;

static const uint16_t dxLeading = 1;	// spacing between chars
static const uint16_t dyLeading = 1;	// spacing between lines of text
static const uint16_t dxMargin = 3;		// margin to leave at left when drawing text
static const uint16_t dyMargin = 2;		// margin to leave at top when drawing text


enum	// LCD Communication Type
{
	lctCmd = 0,		// command
	lctData = 1,	// data
};

#define CMD  0x000
#define DATA 0x100
#define cmd(x)  ((x)|CMD)
#define data(x) ((x)|DATA)

/* Epson S1D15G10 Command Set */
#define DISON       0xaf
#define DISOFF      0xae
#define DISNOR      0xa6
#define DISINV      0xa7
#define COMSCN      0xbb
#define DISCTL      0xca
#define SLPIN       0x95
#define SLPOUT      0x94
#define PASETE      0x75
#define CASETE      0x15
#define DATCTL      0xbc
#define RGBSET8     0xce
#define RAMWRE      0x5c
#define RAMRD       0x5d
#define PTLIN       0xa8
#define PTLOUT      0xa9
#define RMWIN       0xe0
#define RMWOUT      0xee
#define ASCSET      0xaa
#define SCSTART     0xab
#define OSCON       0xd1
#define OSCOFF      0xd2
#define PWRCTR      0x20
#define VOLCTR      0x81
#define VOLUP       0xd6
#define VOLDOWN     0xd7
#define TMPGRD      0x82
#define EPCTIN      0xcd
#define EPCOUT      0xcc
#define EPMWR       0xfc
#define EPMRD       0xfd
#define EPSRRD1     0x7c
#define EPSRRD2     0x7d
#define NOP         0x25

// Philips command set
#define NOPP        0x00
#define BSTRON      0x03
#define SLEEPIN     0x10
#define SLEEPOUT    0x11
#define NORON       0x13
#define INVOFF      0x20
#define INVON       0x21
#define SETCON      0x25
#define DISPOFF     0x28
#define DISPON      0x29
#define CASETP      0x2A
#define PASETP      0x2B
#define RAMWRP      0x2C
#define RGBSET      0x2D
#define MADCTL      0x36
#define COLMOD      0x3A
#define DISCTR      0xB9
#define EC          0xC0

static const uint16_t InitListEpson[] =
    {cmd(DISCTL),  data(0x00), data(0x20), data(0x0a),
     cmd(COMSCN),  data(0x01),
     cmd(OSCON),
     cmd(SLPOUT),
     cmd(VOLCTR),  data(28), data(3),
     cmd(TMPGRD),  data(0),
     cmd(PWRCTR),  data(0x0f),
     cmd(DISINV),
     cmd(PTLOUT),
     // scan cols, then rows, RGB all rows/cols, 8-color
     cmd(DATCTL),  data(0x00), data(0x00), data(0x01), 
	/* if 256-color mode, bytes represent RRRGGGBB; the following
	   maps to 4-bit color for each value in range (0-7 R/G, 0-3 B) */
     cmd(RGBSET8), data(0x00), data(0x02), data(0x04), data(0x06), data(0x08), data(0x0a), data(0x0c), data(0x0f),
                   data(0x00), data(0x02), data(0x04), data(0x06), data(0x08), data(0x0a), data(0x0c), data(0x0f),
                   data(0x00), data(0x06), data(0x09), data(0x0f),
     cmd(DISON)};

static const uint16_t  InitListEpsonLength = sizeof(InitListEpson)/sizeof(InitListEpson[0]);

//!
//! Datasheet url: http://www.nxp.com/documents/data_sheet/PCF8833_1.pdf
//!
static const uint16_t InitListPhilips[] =
    {cmd(SLEEPOUT), cmd(BSTRON), cmd(INVON), 
     cmd(MADCTL), data(0x48), // memory control
     cmd(COLMOD), data(0x02), // 8bpp mode
     cmd(SETCON), data(0x3f), // contrast
     cmd(NOPP),
     cmd(RGBSET),  data(0x00), data(0x02), data(0x04), data(0x06), data(0x08), data(0x0a), data(0x0c), data(0x0f),
                   data(0x00), data(0x02), data(0x04), data(0x06), data(0x08), data(0x0a), data(0x0c), data(0x0f),
                   data(0x00), data(0x06), data(0x09), data(0x0f),
     cmd(DISPON)};

static const uint16_t  InitListPhilipsLength = sizeof(InitListPhilips)/sizeof(InitListPhilips[0]);

#ifdef PHILIPS
#define InitList InitListPhilips
#define InitListLength InitListPhilipsLength

#define CASET CASETP
#define PASET PASETP
#define RAMWR RAMWRP
#else 
#ifdef EPSON
#define InitList InitListEpson
#define InitListLength InitListEpsonLength

#define CASET CASETE
#define PASET PASETE
#define RAWMR RAMWRE
#else
#error unknown LCD type: define PHILIPS or EPSON
#endif
#endif

bool LCD::s_fInit = FALSE;
OUT *LCD::s_poutE;
OUT *LCD::s_poutSCK;
OUT *LCD::s_poutMOSI;
OUT *LCD::s_poutReset;

LCD::LCD(OUT *poutE, OUT *poutMOSI, OUT *poutSCK, OUT *poutReset,
		 const FONT *pfont, uint8_t clrBack, uint8_t clrFore)
		 : m_pfont(pfont),
		   m_clrBack(clrBack),
		   m_clrFore(clrFore),
		   m_x(0),
		   m_y(0)
{
	if (!s_fInit)
	{
		s_poutE = poutE;
		s_poutSCK = poutSCK;
		s_poutMOSI = poutMOSI;
		s_poutReset = poutReset;
		poutE->SetHigh();	// signal is !ChipSelect; this disables it
		poutSCK->SetLow();	// get ready for first send
		Init();
	}

	SetPos(0, 1);	// start at beginning of line 1
}

void LCD::mdelay(uint16_t t) const {
   for (int j = 0; j < t; j++) __asm__ volatile ("nop");
}

void LCD::Init() const
{
	s_fInit = TRUE;

	/* comms with LCD: longest write requirement is 90 ns
	   8 MHz uC executes one instruction every 125 ns */

    s_poutE->SetHigh();
    mdelay(16);
	/* send reset */
	s_poutReset->SetLow();
    mdelay(300);
	s_poutReset->SetHigh();
    mdelay(300);
    
    for (int i = 0; i < InitListLength; i++) {
        WriteCommand(InitList[i]);
    }

	/* clear screen */
	FillRect(0, 0, dxLCDScreen, dyLCDScreen, m_clrBack);
}

/* prints given string on the specified line; unused portion of
   line will be padded with background color by default; use a
   1-based value for a specific line, or 0 for the current line;
   line is updated after outputting text */
void LCD::Print(const char *psz, uint8_t line, bool fPad)
{
	const char *pch;
	uint8_t dy;

	if (m_pfont == NULL)
		return;

	SetPos(0, line);
	pch = psz;
	while (*pch != 0)
		WriteCh(*pch++);
	dy = m_pfont->DyFont() + dyLeading;

	if (fPad && m_clrBack != clrTransparent)	// blank rest of line
	{
		FillRect(0, m_y, dxMargin, dy, m_clrBack);
		FillRect(m_x, m_y, dxLCDScreen - m_x, dy, m_clrBack);
	}

	/* update default output position */
	m_y += dy;
	m_x = dxMargin;
}

/* sets current text output location to the specified position
   (ich, 0-based) on the specified line; note that for proportional
   fonts, horizontal position is an approximation (based on the
   width of a space); using line=0 will leave line unchanged */
void LCD::SetPos(uint8_t ich, uint8_t line)
{
	uint8_t dx;
	uint8_t f;

	if (m_pfont == NULL)
		return;

	if (line != 0)
		m_y = dyMargin + (m_pfont->DyFont() + dyLeading) * (line - 1);

	m_x = dxMargin;
	if (ich > 0)
	{
		/* get width of space */
		m_pfont->PbForCh(' ', &f, &dx);
		m_x += ich * (dx + dxLeading);
	}
}

/* writes a character at the current location, updates current location */
void LCD::WriteCh(char ch)
{
	uint8_t dx, dy;
	uint16_t cpx;
	uint8_t f;
	uint8_t grf;
	const uint8_t *pb;

	if (m_pfont == NULL)
		return;

	dy = m_pfont->DyFont();
	pb = m_pfont->PbForCh(ch, &f, &dx);

	cpx = CpxSetAndValidateLCDRect(m_x, m_y, dx, dy);
	if (cpx != (uint)dx * dy)		// not enough room
		return;

	memcpy(&grf, pb, sizeof(uint8_t));

	if (m_clrBack == clrTransparent)
	{
		uint8_t x, y;

		/* just set desired bits manually */
		for (y = m_y; y < m_y + dy; ++y)
		{
			for (x = m_x; x < m_x + dx; ++x)
			{
				if (grf & f)
					ColorPixel(x, y, m_clrFore);
				if ((f >>= 1) == 0)
				{
					memcpy(&grf, ++pb, sizeof(uint8_t));
					f = 0x80;
				}
			}
		}
		/* update current position */
		m_x += dx + dxLeading;
	}
	else
	{
		/* area set above; just fill in the pixels */
		while (cpx-- > 0)
		{
			WriteCommand(lctData, (grf & f) ? m_clrFore : m_clrBack);
			if ((f >>= 1) == 0)
			{
				memcpy(&grf, ++pb, sizeof(uint8_t));
				f = 0x80;
			}
		}
		/* fill in background for leading below text */
		FillRect(m_x, m_y + dy, dx + dxLeading, dyLeading, m_clrBack);

		/* update current position */
		m_x += dx;
		/* fill in background for leading after character */
		FillRect(m_x, m_y, dxLeading, dy, m_clrBack);
		m_x += dxLeading;
	}
}

/* set the color of given pixel; note that pixels off screen will be ignored */
void LCD::ColorPixel(uint8_t x, uint8_t y, uint8_t clr) const
{
	/* note: this could call FillRect to save some code space, but
	   this is a bit faster (less error-checking needed), and this
	   function is called a lot... */

	if (x >= dxLCDScreen || y >= dyLCDScreen)	// completely off-screen
		return;

	/* convert to LCD coordinates (so the variables really should
	   be xl/yl from here down...) */
	x += xlMin;
	y += ylMin;
	WriteCommand(lctCmd, PASET);	// Page Address Set
	WriteCommand(lctData, y);
	WriteCommand(lctData, y);
	WriteCommand(lctCmd, CASET);	// Column Address Set
	WriteCommand(lctData, x);
	WriteCommand(lctData, x);
	WriteCommand(lctCmd, RAMWR);	// Memory Write
	WriteCommand(lctData, clr);
}

/* draw a solid/filled rectangle with its upper left corner at (x, y),
   with the given width, height, and color */
void LCD::FillRect(uint8_t x, uint8_t y, uint8_t dx, uint8_t dy, uint8_t clr) const
{
	uint16_t cpx;

	cpx = CpxSetAndValidateLCDRect(x, y, dx, dy);
	while (cpx-- > 0)
		WriteCommand(lctData, clr);
}

/* outline a rectangle with its upper left corner at (x, y),
   with the given width, height, and color */
void LCD::FrameRect(uint8_t x, uint8_t y, uint8_t dx, uint8_t dy, uint8_t clr) const
{
	FillRect(x, y, dx, 1, clr);			// top
	FillRect(x, y + dy -1, dx, 1, clr);	// bottom
	FillRect(x, y, 1, dy, clr);			// left
	FillRect(x + dx - 1, y, 1, dy, clr);// right
}

/* helper for DrawLine */
inline void Swap(uint8_t *px, uint8_t *py)
{
	uint8_t t;
	t = *px; *px = *py; *py = t;
}

/* draw a line from (x1, y1) to (x2, y2) */
void LCD::DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t clr) const
{
	uint8_t dx, dy;
	char dyStep;
	bool fMirror;
	int calc;

	/* handle simple horizontal / vertical lines */
	if (x1 == x2)
	{
		if (y1 > y2)	// swap values
			Swap(&y1, &y2);
		FillRect(x1, y1, 1, y2 - y1, clr);
		return;
	}
	if (y1 == y2)
	{
		if (x1 > x2)	// swap values
			Swap(&x1, &x2);
		FillRect(x1, y1, x2 - x1, 1, clr);
		return;
	}

	/* set up parameters */
	dx = Abs((int)x2 - (int)x1);
	dy = Abs((int)y2 - (int)y1);
	/* make sure we travel along longer side; if necessary,
	   we'll flip X and Y (here and below when drawing),
	   effectively mirroring along the line y=x */
	//if ((fMirror = dy > dx) != fFalse)
	if ((fMirror = (dy > dx)))
	{
		/* swap X-Y */
		Swap(&x1, &y1);
		Swap(&x2, &y2);
		Swap(&dx, &dy);
	}
	/* start at the leftmost point */
	if (x1 > x2)
	{
		/* swap points */
		Swap(&x1, &x2);
		Swap(&y1, &y2);
	}
	/* rising or falling? */
	dyStep = y1 < y2 ? 1 : -1;

	/*
	normalized system: line at (0,0) positive slope to (r, s) [geometry coordinates]
		y = mx + b         m = s / r, b = 0     =>   ry = sx
	for each point x along line, y will stay the same as prev or change to y + 1
		ry = sx      or    r (y + 1) = sx
	choose left-side value that is closest to sx; change y if
		r (y + 1) - sx    <    sx - ry
	which simplifies to
		r - 2 (sx - ry) < 0
	the inequality will hold if we divide by two (accounting for integer math),
	so we can use:
		r/2 - sx + ry < 0
	at start, x = y = 0, left side is just r/2
	each iteration, x increases, which subtracts another s (from right side)
	when y increases, that adds another r (to right side)
	*/
	/* draw the line */
	calc = dx >> 1;
	while (x1 <= x2)
	{
		if (fMirror)
			ColorPixel(y1, x1, clr);
		else
			ColorPixel(x1, y1, clr);
		/* move to next x value */
		++x1;
		calc -= dy;
		/* see if should change y value */
		if (calc < 0)
		{
			y1 += dyStep;
			calc += dx;
		}
	}
}

/* calculates 1/8 circle and reflects/rotates to draw full circle */
void LCD::FrameCircle(uint8_t xCenter, uint8_t yCenter, uint8_t r, uint8_t clr) const
{
	uint8_t x, y;
	int calc;

	/* we'll draw pixels starting at (r, 0) and going counter-clockwise
	   (viewed in geometric space, not screen coordinates), rotating
	   and reflecting to fill in 8 sections at once, so we're done
	   when x == y (1/8 of circle calculated).
	   as we traverse that arc, we'll always move either up or up-left,
	   figure out which one is closer to the correct location;
	   change x if:
		  r^2 - left pixel x^2+y^2   <    right pixel x^2+y^2 - r^2
		  r^2 - ((x - 1)^2 + y^2)    <    x^2 + y^2 - r^2
		  r2 - (x2 - 2*x + 1 + y2)   <    x2 + y2 - r2
		  r2 - x2 - y2 + 2x - 1      <    x2 + y2 - r2
		  2r2 - 2x2 - 2y2 + 2x       <    1
		  r2 - x2 - y2 + x           <    1/2
		     (only int < 1/2 is 0 or negative)
		  r2 - x2 - y2 + x           <=   0
	   at beginning, x = r, y = 0, so left side is just x
	   when increment y, y^2 term increases by 2y-1 (so term decreases)
	   when decrement x, x^2 term decreases by 2x+1 (so term increases)
	   */

	x = r;
	y = 0;
	calc = x;

	while (x >= y)
	{
		/* set pixel at current locations */
		/* (negative values will become large pos numbers and get rejected) */
		ColorPixel(xCenter + x, yCenter + y, clr);
		ColorPixel(xCenter + x, yCenter - y, clr);
		ColorPixel(xCenter - x, yCenter + y, clr);
		ColorPixel(xCenter - x, yCenter - y, clr);
		ColorPixel(xCenter + y, yCenter + x, clr);
		ColorPixel(xCenter + y, yCenter - x, clr);
		ColorPixel(xCenter - y, yCenter + x, clr);
		ColorPixel(xCenter - y, yCenter - x, clr);
		/* should next pixel be up or up-left? */
		++y;			// move up
		calc -= 2 * y - 1;
		if (calc <= 0)	// move left?
		{
			--x;
			calc += 2 * x + 1;
		}
	}
}

/* calculates 1/8 circle and reflects/rotates to draw full circle;
   see FrameCircle for calc explanation */
void LCD::FillCircle(uint8_t xCenter, uint8_t yCenter, uint8_t r, uint8_t clr) const
{
	uint8_t x, y;
	int calc;
	uint8_t xT, d;

	x = r;
	y = 0;
	calc = x;

	while (x >= y)
	{
		/* if circle extends off left side, need to handle it here (no
		   negatives, so those values become large pos values and get
		   rejected by FillRect) */
		if (x <= xCenter)
		{
			xT = xCenter - x;
			d = 2 * x;
		}
		else
		{
			xT = 0;
			/* 2x - (x - xCenter) = x + xCenter */
			d = x + xCenter;
		}
		FillRect(xT, yCenter + y, d, 1, clr);
		FillRect(xT, yCenter - y, d, 1, clr);

		if (y <= xCenter)
		{
			xT = xCenter - y;
			d = 2 * y;
		}
		else
		{
			xT = 0;
			/* 2y - (y - xCenter) = y + xCenter */
			d = y + xCenter;
		}
		FillRect(xT, yCenter + x, d, 1, clr);
		FillRect(xT, yCenter - x, d, 1, clr);

		/* should next pixel be up or up-left? */
		++y;			// move up
		calc -= 2 * y - 1;
		if (calc <= 0)	// move left?
		{
			--x;
			calc += 2 * x + 1;
		}
	}
}

/* draw the specified bitmap (located in flash memory) at the given
   coordinates (upper left corner); the first two bytes of the memory
   specify the width and height, with remaining bytes as 8-bit color
   values; will not draw anything if not enough room for full image */
void LCD::ShowBitmap(const uint8_t *pab, uint8_t x, uint8_t y) const
{
	uint8_t dz[2], clr;
	uint16_t cpx;

	memcpy(dz, pab, 2 * sizeof(uint8_t));
	pab += 2;
	cpx = CpxSetAndValidateLCDRect(x, y, dz[0], dz[1]);
	if (cpx != (uint)dz[0] * dz[1])
		return;

	while (cpx-- > 0)
	{
		memcpy(&clr, pab++, sizeof(uint8_t));
		WriteCommand(lctData, clr);
	}
}

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

void LCD::SetVolume(uint8_t ratio, uint8_t volume) const
{
#ifdef PHILIPS
    WriteCommand(lctCmd, SETCON);
    WriteCommand(lctData, volume & 0x7f);
#else
	WriteCommand(lctCmd, VOLCTR);	// Electronic Volume Control (LCD brightness)
	WriteCommand(lctData, volume & 0x3f);
	WriteCommand(lctData, ratio & 0x03);
#endif
}

void LCD::IncVolumn() const
{
	WriteCommand(lctCmd, VOLUP);	// Increment Electronic Control
}

void LCD::DecVolumn() const
{
	WriteCommand(lctCmd, VOLDOWN);	// Decrement Electronic Control
}

/*
*	helper functions
*/

void LCD::WriteCommand(uint16_t data) const {
    s_poutE->SetLow();  // enable chip

    for (uint8_t i = 0; i < 9; i++) {
        if ((data & 0x100) == 0x100) {
            s_poutMOSI->SetHigh();
        } else {
            s_poutMOSI->SetLow();
        }

        s_poutSCK->SetHigh();
        s_poutSCK->SetLow();

        data <<= 1;
    }
    s_poutE->SetHigh();
}

/* send an instruction to the LCD */
void LCD::WriteCommand(uint8_t lct, uint8_t ch) const
{
    WriteCommand(ch | (lct<<8));
#if 0
	/* enable chip */
	s_poutE->SetLow();

	/* send first bit manually */
	/* set data bit for command or data */
	if (lct == lctCmd)
		s_poutMOSI->SetLow();
	else
		s_poutMOSI->SetHigh();
	/* pulse clock */
	s_poutSCK->SetHigh();
	asm("nop");		// just to be sure...
	s_poutSCK->SetLow();

	/* enable SPI for remaining 8 bits */
	SetBit(regSPCR, bitSPE);
	regSPDR = ch;
	/* wait for transmission to complete */
	while (bit_is_clear(regSPSR, bitSPIF))
		;
	ClearBit(regSPCR, bitSPE);	// disable it so we can send manually...
	
	/* signal done with this transmit */
	s_poutE->SetHigh();
#endif
}

/* ensures that rectangle is fully on screen and sets LCD for drawing there;
   returns number of pixels available to fill */
uint16_t LCD::CpxSetAndValidateLCDRect(uint8_t x, uint8_t y, uint8_t dx, uint8_t dy) const
{
	uint8_t xlFirst, ylFirst, xlLast, ylLast;	// LCD coordinates

	/* check upper left corner */
	/* (x and y aren't too low since unsigned can't be < 0!) */
	if (x >= dxLCDScreen || y >= dyLCDScreen)	// completely off-screen
		return 0;

	/* check lower right corner */
	if (x + dx > dxLCDScreen)
		dx = dxLCDScreen - x;
	if (y + dy > dyLCDScreen)
		dy = dyLCDScreen - y;

	/* convert to LCD coordinates */
	xlLast = (xlFirst = xlMin + x) + dx - 1;
	ylLast = (ylFirst = ylMin + y) + dy - 1;

	/* note: for PASET/CASET, docs say that start must be < end,
	   but <= appears to be OK; end is a "last" not "lim" value */
	WriteCommand(lctCmd, PASET);	// Page Address Set
	WriteCommand(lctData, ylFirst);	// start page (line)
	WriteCommand(lctData, ylLast);	// end page
	WriteCommand(lctCmd, CASET);	// Column Address Set
	WriteCommand(lctData, xlFirst);	// start address
	WriteCommand(lctData, xlLast);	// end address
	WriteCommand(lctCmd, RAMWR);	// Memory Write

	return (uint)dx * dy;
}
