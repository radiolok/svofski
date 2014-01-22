/*
   Copyright (c) 1992 - 1994 Heinz W. Werntges.  All rights reserved.
   Parts Copyright (c) 1999  Martin Kroeker  All rights reserved.
   
   Distributed by Free Software Foundation, Inc.

This file is part of HP2xx.

HP2xx is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the GNU General Public License, Version 2 or later, for full details.

Everyone is granted permission to copy, modify and redistribute
HP2xx, but only under the conditions described in the GNU General Public
License.  A copy of this license is supposed to have been
given to you along with HP2xx so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*/

/**
 ** This file defines a standard character set by elementary
 ** "draw" & "move" commands. The format is a very compact one from
 ** the old days where every byte was still appreciated.
 **
 ** A font or character set is an array of strings. Each character
 ** corresponds to one of these strings, which is addressed by its ASCII code.
 **
 ** A character is a (NULL-terminated) string of bytes. Each byte
 ** codes for a draw or move action according to the code below:
 **
 **	Bit:	7 6 5 4 3 2 1 0
 **		p x x x y y y y
 **
 **	p:	Plot flag. If set, "draw to" new point, else "move to" it.
 **	xxx:	3-bit unsigned integer  (0...7). X coordinate of new point.
 **	yyyy:	4-bit unsigned integer (0..15). Y coordinate of new point.
 **
 ** The baseline is y = 4 instead of y = 0, so characters with parts
 ** below it can be drawn properly without a need for sign bits.
 ** Function "code_to_ucoord()" transforms these coordinates into
 ** actual user coordinates.
 **
 ** Example:	code for character 'L': "\032\224\324" translates to:
 **		moveto(1,10); drawto(1,4); drawto(5,4);
 **
 ** From the example you can conclude that the font below essentially is
 ** defined on a 5x7 grid:
 **
 **	  	0 1 2 3 4 5 6 7
 **	15	. . . . . . . .		. : unused
 **	14	. . . . . . . .		* : always used
 **	13	. . . . . . . .		o : sometimes used
 **	12	. . . . . . . .
 **	11	. . . . . . . .
 **	10	o * * * * * . .
 **	 9	o * * * * * . .
 **	 8	o * * * * * . .
 **	 7	o * * * * * . .
 **	 6	o * * * * * . .
 **	 5	o * * * * * . .
 **	 4	o * * * * * . .
 **	 3	o o o o o o . .
 **	 2	o o o o o o . .
 **	 1	o o o o o o . .
 **	 0	o o o o o o . .
 **/


/**
 ** The following array of strings contains the basic character set (set 0).
 **
 ** NOTE: A nice way to add a new charset would be, e. g., to introduce a
 ** ``charset1[]'' as the "alternate" charset and implement the HP-GL
 ** commands needed for switching from one to the other.
 **/

#include <avr/pgmspace.h>

	/* 0x00 ... 0x1f        */

/**
 ** Unfortunately, some compilers do not process \xNN properly,
 ** so I changed all hex codes (\xNN) into octal codes (\NNN),
 ** thereby losing readability but gaining portability.
 **/

	/* 0x20 ... 0x2f        */
const char chr20[] PROGMEM =	"";
const char chr21[] PROGMEM =	"\064\265\066\272";
const char chr22[] PROGMEM =	"\051\252\111\312";
const char chr23[] PROGMEM =	"\044\252\104\312\026\326\030\330";
const char chr24[] PROGMEM =	"\064\272\131\251\230\247\307\326\305\225";
const char chr25[] PROGMEM =	"\024\332\051\250\270\271\251\066\265\305\306\266";
const char chr26[] PROGMEM =	"\124\230\231\252\271\270\226\225\244\264\326";
const char chr27[] PROGMEM =	"\071\312";
const char chr28[] PROGMEM =	"\132\270\266\324";
const char chr29[] PROGMEM =	"\024\266\270\232";
const char chr2a[] PROGMEM =	"\005\351\145\211\072\264";
const char chr2b[] PROGMEM =	"\065\271\027\327";
const char chr2c[] PROGMEM =	"\064\244\245\265\263\242";
const char chr2d[] PROGMEM =	"\027\327";
const char chr2e[] PROGMEM =	"\064\244\245\265\264";
const char chr2f[] PROGMEM =	"\352";

	/* 0x30 ... 0x3f        */
/*
"\025\244\304\325\331\312\252\231\225\331", ** Zero including `/' **
*/
const char chr30[] PROGMEM =	"\025\244\304\325\331\312\252\231\225";
const char chr31[] PROGMEM =	"\044\304\064\272\251";
const char chr32[] PROGMEM =	"\031\252\312\331\330\225\224\324";
const char chr33[] PROGMEM =	"\025\244\304\325\326\307\267\332\232";
const char chr34[] PROGMEM =	"\112\227\226\326\107\304";
const char chr35[] PROGMEM =	"\132\232\230\310\327\325\304\244\225";
const char chr36[] PROGMEM =	"\132\272\230\225\244\304\325\326\307\227";
const char chr37[] PROGMEM =	"\032\332\331\226\224";
const char chr38[] PROGMEM =	"\107\330\331\312\252\231\230\247\307\326\325\304\244\225\226\247";
const char chr39[] PROGMEM =	"\044\264\326\331\312\252\231\230\247\327";
const char chr3a[] PROGMEM =	"\047\250\270\267\247\045\265\264\244\245";
const char chr3b[] PROGMEM =	"\046\247\267\266\246\064\244\245\265\263\242";
const char chr3c[] PROGMEM =	"\112\227\304";
const char chr3d[] PROGMEM =	"\030\330\026\326";
const char chr3e[] PROGMEM =	"\032\307\224";
const char chr3f[] PROGMEM =	"\031\252\312\331\330\307\267\266\065\264";

	/* 0x40 ... 0x4f        */
const char chr40[] PROGMEM =	"\103\243\224\230\252\312\331\326\305\266\267\310\330";
const char chr41[] PROGMEM =	"\024\231\252\312\331\324\026\326";
const char chr42[] PROGMEM =	"\024\232\312\331\330\307\227\024\304\325\326\307";
const char chr43[] PROGMEM =	"\125\304\244\225\231\252\312\331";
const char chr44[] PROGMEM =	"\024\232\312\331\325\304\224";
const char chr45[] PROGMEM =	"\124\224\232\332\027\307";
const char chr46[] PROGMEM =	"\024\232\332\027\307";
const char chr47[] PROGMEM =	"\131\312\252\231\225\244\304\325\327\247";
const char chr48[] PROGMEM =	"\024\232\124\332\027\327";
const char chr49[] PROGMEM =	"\024\324\064\272\032\332";
const char chr4a[] PROGMEM =	"\025\244\304\325\332\232";
const char chr4b[] PROGMEM =	"\024\232\027\247\324\047\332";
const char chr4c[] PROGMEM =	"\032\224\324";
const char chr4d[] PROGMEM =	"\024\232\270\332\324";
const char chr4e[] PROGMEM =	"\024\232\324\332";
const char chr4f[] PROGMEM =	"\044\225\231\252\312\331\325\304\244";

	/* 0x50 ... 0x5f        */
const char chr50[] PROGMEM =	"\024\232\312\331\330\307\227";
const char chr51[] PROGMEM =	"\044\225\231\252\312\331\326\264\244\066\324";
const char chr52[] PROGMEM =	"\024\232\312\331\330\307\227\247\324";
const char chr53[] PROGMEM =	"\025\244\304\325\326\307\247\230\231\252\312\331";
const char chr54[] PROGMEM =	"\064\272\232\332";
const char chr55[] PROGMEM =	"\032\225\244\304\325\332";
const char chr56[] PROGMEM =	"\032\230\264\330\332";
const char chr57[] PROGMEM =	"\032\224\267\324\332";
const char chr58[] PROGMEM =	"\024\332\124\232";
const char chr59[] PROGMEM =	"\032\231\266\264\066\331\332";
const char chr5a[] PROGMEM =	"\032\332\224\324";
const char chr5b[] PROGMEM =	"\124\264\272\332";
const char chr5c[] PROGMEM =	"\032\324";
const char chr5d[] PROGMEM =	"\024\264\272\232";
const char chr5e[] PROGMEM =	"\030\272\330";
const char chr5f[] PROGMEM =	"\023\323";

	/* 0x60 ... 0x6f        */
const char chr60[] PROGMEM =	"\053\310";
const char chr61[] PROGMEM =	"\124\244\225\227\250\310\304";
const char chr62[] PROGMEM =	"\024\304\325\327\310\250\052\244";
const char chr63[] PROGMEM =	"\125\304\264\245\247\270\310\327";
const char chr64[] PROGMEM =	"\112\304\244\225\227\250\310\104\324";
const char chr65[] PROGMEM =	"\026\306\327\310\250\227\225\244\324";
const char chr66[] PROGMEM =	"\064\271\312\332\047\307";
const char chr67[] PROGMEM =	"\022\262\303\310\250\227\225\244\304";
const char chr68[] PROGMEM =	"\032\224\030\270\307\304";
const char chr69[] PROGMEM =	"\072\271\050\270\264\044\304";
const char chr6a[] PROGMEM =	"\072\271\050\270\263\242\222";
const char chr6b[] PROGMEM =	"\024\232\104\226\310";
const char chr6c[] PROGMEM =	"\052\272\264\044\304";
const char chr6d[] PROGMEM =	"\024\230\027\250\267\264\067\310\327\324";
const char chr6e[] PROGMEM =	"\024\230\027\250\270\307\304";
const char chr6f[] PROGMEM =	"\044\225\227\250\270\307\305\264\244";

	/* 0x70 ... 0x7f        */
const char chr70[] PROGMEM =	"\022\230\270\307\305\264\224";
const char chr71[] PROGMEM =	"\104\244\225\227\250\310\302";
const char chr72[] PROGMEM =	"\030\224\026\270\310";
const char chr73[] PROGMEM =	"\110\250\227\246\266\305\264\224";
const char chr74[] PROGMEM =	"\052\244\304\030\310";
const char chr75[] PROGMEM =	"\030\225\244\304\310";
const char chr76[] PROGMEM =	"\030\226\264\326\330";
const char chr77[] PROGMEM =	"\030\225\244\265\267\065\304\325\330";
const char chr78[] PROGMEM =	"\030\324\024\330";
const char chr79[] PROGMEM =	"\022\326\330\030\226\264";
const char chr7a[] PROGMEM =	"\030\310\224\304";
const char chr7b[] PROGMEM =	"\113\273\252\250\227\246\244\263\303";
const char chr7c[] PROGMEM =	"\073\263";
const char chr7d[] PROGMEM =	"\053\273\312\310\327\306\304\263\243";
const char chr7e[] PROGMEM =	"\031\252\310\331";
const char chr7f[] PROGMEM =	"";

PGM_P const charset0[256] PROGMEM = {
 chr20,
 chr21,
 chr22,
 chr23,
 chr24,
 chr25,
 chr26,
 chr27,
 chr28,
 chr29,
 chr2a,
 chr2b,
 chr2c,
 chr2d,
 chr2e,
 chr2f,

	/* 0x30 ... 0x3f        */
/*
"\025\244\304\325\331\312\252\231\225\331", ** Zero including `/' **
*/
 chr30,
 chr31,
 chr32,
 chr33,
 chr34,
 chr35,
 chr36,
 chr37,
 chr38,
 chr39,
 chr3a,
 chr3b,
 chr3c,
 chr3d,
 chr3e,
 chr3f,

	/* 0x40 ... 0x4f        */
 chr40,
 chr41,
 chr42,
 chr43,
 chr44,
 chr45,
 chr46,
 chr47,
 chr48,
 chr49,
 chr4a,
 chr4b,
 chr4c,
 chr4d,
 chr4e,
 chr4f,

	/* 0x50 ... 0x5f        */
 chr50,
 chr51,
 chr52,
 chr53,
 chr54,
 chr55,
 chr56,
 chr57,
 chr58,
 chr59,
 chr5a,
 chr5b,
 chr5c,
 chr5d,
 chr5e,
 chr5f,

	/* 0x60 ... 0x6f        */
 chr60,
 chr61,
 chr62,
 chr63,
 chr64,
 chr65,
 chr66,
 chr67,
 chr68,
 chr69,
 chr6a,
 chr6b,
 chr6c,
 chr6d,
 chr6e,
 chr6f,

	/* 0x70 ... 0x7f        */
 chr70,
 chr71,
 chr72,
 chr73,
 chr74,
 chr75,
 chr76,
 chr77,
 chr78,
 chr79,
 chr7a,
 chr7b,
 chr7c,
 chr7d,
 chr7e,
 chr7f,

 chr20,
 chr21,
 chr22,
 chr23,
 chr24,
 chr25,
 chr26,
 chr27,
 chr28,
 chr29,
 chr2a,
 chr2b,
 chr2c,
 chr2d,
 chr2e,
 chr2f,

	/* 0x30 ... 0x3f        */
/*
"\025\244\304\325\331\312\252\231\225\331", ** Zero including `/' **
*/
 chr30,
 chr31,
 chr32,
 chr33,
 chr34,
 chr35,
 chr36,
 chr37,
 chr38,
 chr39,
 chr3a,
 chr3b,
 chr3c,
 chr3d,
 chr3e,
 chr3f,

	/* 0x40 ... 0x4f        */
 chr40,
 chr41,
 chr42,
 chr43,
 chr44,
 chr45,
 chr46,
 chr47,
 chr48,
 chr49,
 chr4a,
 chr4b,
 chr4c,
 chr4d,
 chr4e,
 chr4f,

	/* 0x50 ... 0x5f        */
 chr50,
 chr51,
 chr52,
 chr53,
 chr54,
 chr55,
 chr56,
 chr57,
 chr58,
 chr59,
 chr5a,
 chr5b,
 chr5c,
 chr5d,
 chr5e,
 chr5f,

	/* 0x60 ... 0x6f        */
 chr60,
 chr61,
 chr62,
 chr63,
 chr64,
 chr65,
 chr66,
 chr67,
 chr68,
 chr69,
 chr6a,
 chr6b,
 chr6c,
 chr6d,
 chr6e,
 chr6f,

	/* 0x70 ... 0x7f        */
 chr70,
 chr71,
 chr72,
 chr73,
 chr74,
 chr75,
 chr76,
 chr77,
 chr78,
 chr79,
 chr7a,
 chr7b,
 chr7c,
 chr7d,
 chr7e,
 chr7f,
};
