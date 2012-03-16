#include <tonc_gba.h>
#include <luts.h>
#include "text.h"
#include "fonts/font_8x8.h"

typedef struct _wtf_letter {
	u8 x, y;
	u8 orig_x, orig_y;
	u8 allowance;
} LetterView;

OBJINFO* oi_font8x8 = &oi_buffer[FONT8X8_OAMBASE];

static LetterView text_letterLocations[64];

void font_load8x8() {
	int i;

	// let's keep our font in 64 8x8 tiles starting from 16th
	dma_memcpy(&tl_mem[TB_OBJ][FONT8X8_TILEBASE], font_8x8Data, font_8x8Len/4);

	for (i = 0; i < 64; i++) {
		oi_set_pos(&oi_font8x8[i], i*8, 24);
	}
	oam_update_all();
}

void text_display_char(int nchar, char asciival) {
	u8 value = asciival - 32;
	if (value > 64) {
		value = 0;
	}
	oi_set_attr(&oi_font8x8[nchar], OI_A0_PAL256, OI_A1_SIZE_8, 128+value*2);
	oam_update(FONT8X8_OAMBASE+nchar, 1);
}

void text_move_char(int nchar, int x, int y) {
	oi_set_pos(&oi_font8x8[nchar], x, y);
	text_letterLocations[nchar].x = x;
	text_letterLocations[nchar].y = y;
	oam_update(FONT8X8_OAMBASE+nchar, 1);
}

void text_display_string(int charbase, char *string, int x, int y) {
	int i;
	int ofs;
	u8 value;

	for (i = 0; i < 32 && *string != 0; i++) {
		value = *string++-32;
		if (value > 64) {
			value = 0;
		}
		ofs = i + charbase;
		oi_set_attr(&oi_font8x8[ofs], OI_A0_PAL256, OI_A1_SIZE_8, 128+value*2);
		oi_set_pos(&oi_font8x8[ofs], x, y);

		LetterView *view = &text_letterLocations[ofs];
		view->x = view->orig_x = x;
		view->y = view->orig_y = y;
		
		x+=8;
	}

	for (; i < 32;i++) {
		ofs = i + charbase;
		oi_set_attr(&oi_font8x8[ofs], OI_A0_PAL256, OI_A1_SIZE_8, 128);
		oi_set_pos(&oi_font8x8[ofs], 250, 250);
		LetterView *view = &text_letterLocations[ofs];
		view->x = view->orig_x = 250;
		view->y = view->orig_y = 250;
	}

	oam_update(FONT8X8_OAMBASE+charbase, 32);
}

void text_display_string_one(char *string, int x, int y) {
	text_display_string(0, string, x, y);
}

void text_display_string_two(char *string, int x, int y) {
	text_display_string(32, string, x, y);
}

void text_move_rel(int nchar, int dx, int dy) {
	LetterView *view = &text_letterLocations[nchar];

	view->x += dx;
	view->y += dy;

	oi_set_pos(&oi_font8x8[nchar], view->x, view->y);
}

void text_set_allowance(int nchar, int length, int allowance) {
	int ii;
	for (ii = nchar; ii < nchar+length; ii++) {
		text_letterLocations[nchar].allowance = allowance;
	}
}

inline void text_clamp_position(int nchar) {
	LetterView *view = &text_letterLocations[nchar];
	register u8 allo = view->allowance;
	if (view->x > view->orig_x + allo) {
		view->x = view->orig_x;
	}
	if (view->x < view->orig_x - allo) {
		view->x = view->orig_x;
	}
	if (view->y > view->orig_y + allo) {
		view->y = view->orig_y;
	}
	if (view->y < view->orig_y - allo) {
		view->y = view->orig_y;
	}
}
