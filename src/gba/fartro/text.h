#ifndef _FONTss_H
#define _FONTss_H

#define FONT8X8_TILEBASE 128	// starting tile
#define FONT8X8_OAMBASE 16	// starting OAM

void font_load8x8();
void text_display_char(int nchar, char asciival);
void text_move_char(int nchar, int x, int y);

void text_display_string(int charbase, char *string, int x, int y);
void text_display_string_one(char *string, int x, int y);
void text_display_string_two(char *string, int x, int y);
void text_move_rel(int nchar, int dx, int dy);
void text_set_allowance(int nchar, int howmanychars, int allowance);
void text_clamp_position(int nchar);

#endif
