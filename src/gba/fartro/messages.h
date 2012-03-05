#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include <tonc_gba.h>

typedef struct _text_string {
	char *text;
	u8 x;
	u8 y;
	u32 start_frame;
	u32 display_time;
} TextString;

extern TextString text_strings[];

#endif
