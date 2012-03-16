/*	Fonts.h
*
*	Bitmap fonts, stored in flash memory.
*
*	Define any/all of the following for access to the corresponding font:
*
*		                type           height   width
*		                ------------   ------   ------------
*		FONT_5x9		proportional     9      1-5 (most 5)
*		FONT_5x9_MONO	monospace        9      5
*		FONT_3x5		proportional     5      1-5 (most 3)
*
*	Revisions:
*		07-13-06	included in LCDSample project
*		07-11-06	version 1 in master library
*
*	Written by Cathy Saxton
*	robotics@idleloop.com
*/

#pragma once

#include <inttypes.h>

class FONT
{
public:
	FONT(uint8_t dyFont, uint8_t cch, const uint8_t *papx, const uint16_t *pmpchx);

	uint8_t DyFont() const	{ return m_dyFont; }

	/* returns first byte containing pixels for ch, and sets
	   *pf to mask its bit and *pdx to the width of the char
	   (pixels are ordered left to right, top to bottom) */
	const uint8_t *PbForCh(uint8_t ch, uint8_t *pf, uint8_t *pdx) const;

private:
	uint8_t DxForIch(uint8_t ich, uint16_t *pcbit) const;

	uint8_t m_dyFont;		// height of each character
	uint8_t m_cch;		// num of chars for which we have bitmaps (starting with ' ')
	const uint8_t *m_papx;	// (flash) array of pixels
	const uint16_t *m_pmpchx;	// (flash) array of starting column for each char
};

#ifdef FONT_5x9
extern FONT g_font5x9;
#endif
#ifdef FONT_5x9_MONO
extern FONT g_font5x9Mono;
#endif
#ifdef FONT_3x5
extern FONT g_font3x5;
#endif
