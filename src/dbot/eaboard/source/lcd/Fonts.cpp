/*	Fonts.cpp
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

#include <inttypes.h>

#include "common.h"

#include "Fonts.h"


/*	storage...

	font pixels are stored as follows...

	Each character's pixels are stored left to right across each row,
	top to bottom. Characters are packed next to each other (not byte-
	aligned). Characters for which there is no representation are skipped.

	Each font has an array holding the starting column (0-based) for each
	character (starting with space).
*/


FONT::FONT(uint8_t dyFont, uint8_t cch, const uint8_t *papx, const uint16_t *pmpchx)
		   : m_dyFont(dyFont),
		     m_cch(cch),
			 m_papx(papx),
			 m_pmpchx(pmpchx)
{
}

/* returns first byte containing pixels for ch, and sets
   *pf to mask its bit and *pdx to the width of the char
   (pixels are ordered left to right, top to bottom) */
const uint8_t *FONT::PbForCh(uint8_t ch, uint8_t *pf, uint8_t *pdx) const
{
	uint8_t ich;
	uint16_t cbit;

	/* make sure given ch in range, else use space */
	if (ch < ' ' || (ich = ch - ' ') >= m_cch)
		*pdx = DxForIch(0, &cbit);
	else
	{
		/* get char width offset and bit offset */
		*pdx = DxForIch(ich, &cbit);
		if (*pdx == 0)
		{
			/* no bitmap for this char; pick an alternative;
			   try uppercase for lowercase char, else use space */
			if (ch < 'a' || ch > 'z'
				|| (*pdx = DxForIch(ich - ('a'-'A'), &cbit)) == 0)
			{
				/* use space instead of unknown/unmapped character */
				*pdx = DxForIch(0, &cbit);
			}
		}
	}
	/* bits: 01234567 89... */
	*pf = 0x80 >> (cbit % 8);
	return &m_papx[cbit / 8];
}

/* returns width of specified char (0 for unmapped char)
   and for non-0-width chars, sets *pcbit to the bit offset
   for this char in pixel bitmap */
uint8_t FONT::DxForIch(uint8_t ich, uint16_t *pcbit) const
{
	uint16_t ax[2];
	uint16_t dx;

    ax[0] = m_pmpchx[ich];
    ax[1] = m_pmpchx[ich+1];

	if ((dx = ax[1] - ax[0]) == 0)
		return 0;
	*pcbit = ax[0] * m_dyFont;
	return (uint8_t)dx;
}

#ifdef FONT_5x9
#include "5x9.txt"
#endif	// FONT_5x9

#ifdef FONT_5x9_MONO
#include "5x9Mono.txt"
#endif	// FONT_5x9_MONO

#ifdef FONT_3x5
#include "3x5.txt"
#endif	// FONT_3x5
