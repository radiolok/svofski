#include <stdio.h>
#include <stdlib.h>

#include <inttypes.h>

#include "vectrex.h"
#include "pecado.h"


#define ITEMS_PER_PAGE	5
#define XTHRESHOLD 		14
#define YTHRESHOLD 		8
#define PAGE_LEFT (-40)

typedef int (*FRAMEFUNC)(int);

const int8_t boxpath[] = {
   0,   -10,-10,    
   255,  10,-10,    
   255,  10, 10,
   255, -10, 10,
   255, -10,-10, 
   1};

// const int8_t starpath[] = {
// 	0, 9, 3,
// 	255, -6,-8,
// 	255,  0,10,
// 	255,  6,-8,
// 	255,  -9, 3,
// 	255,  9, 3,
// 	1};

const int8_t starpath[] = {
	0,   0,	120,
	255, -32, 37,
	255, -122, 32,
	255, -53, -25,
	255, -75, -112,
	255, 0, -65,
	255, 76, -112,
	255, 55, -26,
	255, 122, 31,
	255, 34, 38, 
	255, 0, 120,
	1
	// 0,   18,	6,
	// 255, 5, 6,
	// 255, 0, 20,
	// 255, -5, 6,
	// 255, -18, 6,
	// 255, -8, -4,
	// 255, -12, -16,
	// 255, 0, -8,
	// 255, 12, -16,
	// 255, 8, -4, 
	// 255, 18, 6,
	// 1
};

int8_t starrot[sizeof(starpath)];

void SetCharSize(uint8_t w, uint8_t h) {
	*(uint8_t *)0xc82a = h;
	*(uint8_t *)0xc82b = w;
}

void SetCharSizeHW(uint16_t hw) {
	*(uint16_t *)0xc82a = hw;
}

const char hexchar[] = "0123456789ABCDEF";

void itohex8(char* buf, uint8_t val) {
	*buf++ = hexchar[val >> 4];
	*buf++ = hexchar[val & 0x0f];
}

const char* names[] = {
	"TITLE1\200", 
	"TITLE2\200",
	"TITLE3\200",
	"TITLE4\200",
	"TITLE5\200",
	"TITLE6\200",
	"TITLE7\200",
	"TITLE8\200",
	"TITLE9\200",
	"TITLE10\200",
	"TITLE11\200", 
	"TITLE12\200",
	"TITLE13\200",
	// "TITLE14\200",
	// "TITLE15\200",
	// "TITLE16\200",
	// "TITLE17\200",
	// "TITLE18\200",
	// "TITLE19\200",
	// "TITLE20\200",
};

typedef struct zoomani_ {
	uint16_t zoom;
	int8_t xofs;
	int8_t yofs;
	uint8_t intensity;
} ZoomDesc;

const ZoomDesc selectzoom[] = {
	{zoom:0xf850, xofs:0, yofs:0, intensity:0x20},
	{zoom:0xf854, xofs:0, yofs:0, intensity:0x42},
	{zoom:0xf858, xofs:0, yofs:0, intensity:0x58},
	{zoom:0xf75c, xofs:-2, yofs:0, intensity:0x6c},
	{zoom:0xf760, xofs:-2, yofs:1, intensity:0x70},
	{zoom:0xf664, xofs:-4, yofs:1, intensity:0x78},
	{zoom:0xf568, xofs:-4, yofs:1, intensity:0x7f},
};

volatile int8_t aniframe, anix, aniy;

void animate_selected_start() {
	aniframe = 255;
}

void animate_selected() {
	aniframe++;

	if (aniframe >= sizeof(selectzoom)/sizeof(selectzoom[0])) {
		aniframe = sizeof(selectzoom)/sizeof(selectzoom[0]) - 1;
	}
	SetCharSizeHW(selectzoom[aniframe].zoom);
	anix = selectzoom[aniframe].xofs;
	aniy = selectzoom[aniframe].yofs;
	Intensity(selectzoom[aniframe].intensity);
}


#define min(a,b) ((a)<(b)?(a):(b))

int8_t starspeed;
int16_t star_y;
uint8_t starframe;
int8_t selected;
int8_t page_start;
uint8_t total_items;
int8_t page_boundary;

int8_t page_left;
int16_t page_left_current;
int16_t page_left_goal;
uint8_t xthreshold;
uint8_t ythreshold;

void Star_Animate(uint8_t frame) {
	int16_t last_x = 0, last_y = 0;

	for (int i = 0; i < sizeof(starpath)/sizeof(starpath[0]) - 3; i += 3) {
		starrot[i] = starpath[i];
		int16_t x = starpath[i+1];
		int16_t y = starpath[i+2];
		irotate0(&x, &y, frame);
        starrot[i+1] = y - last_y;
        starrot[i+2] = x - last_x;
        last_y = y;
        last_x = x;	
    }
    starrot[sizeof(starrot)/sizeof(starrot[0]) - 1] = 1;
}


void MainFrameReset() {
	total_items = sizeof(names)/sizeof(names[0]);
	page_boundary = min(ITEMS_PER_PAGE, sizeof(names)/sizeof(names[0]));
}

void MainFrameInit() {
	starspeed = 0;
	page_left = PAGE_LEFT;
	xthreshold = XTHRESHOLD;
}

int8_t drawPageItems() 
{
	int8_t sel_y = 0;

	for (int8_t i = page_start, y = 80; i < page_boundary; i++, y -= 220/ITEMS_PER_PAGE) {
		const char *title = names[i];
		if (i - page_start == selected) {
			animate_selected();

			Print_Str_d(page_left + anix, y + aniy, title);
			//Print_Str_d(page_left + anix, y + 1 + aniy, title);
			sel_y = y;
		} else {
			Intensity(0x60);
			SetCharSizeHW(0xf850);
			Print_Str_d(page_left, y, title);
		}
	}

	return sel_y;
}

int FlipPageInit(uint8_t forward) {
	page_left_current = forward ? 128 * 128 : -128 * 128;
	page_left_goal = PAGE_LEFT*128;

	return 1;
}

int FlipPage(int frame) {
	int16_t error = (page_left_goal - page_left_current) / 6;
	page_left_current += error;

	page_left = page_left_current/128;

	return (abs(error) < 128) ? 0 : 1;
}

int MainFrame(int frame) {
	static uint8_t flipping = 0;
	static uint8_t funprohibited;
	int8_t advance = 0;

	if (funprohibited > 0) --funprohibited;

	if (funprohibited == 0) {
		if (joystick1_y < -32) {
			if (ythreshold > 0) {
				--ythreshold;
				star_y -= 256;
			}
			if (ythreshold == 0 || joystick1_y <= -64) {
				ythreshold = YTHRESHOLD;
				selected++;
				advance = 1;
				funprohibited = 10;
			}
		} 
		else if (joystick1_y > 32) {
			if (ythreshold > 0) {
				--ythreshold;
				star_y += 256;;
			}
			if (ythreshold == 0 || joystick1_y >= 64) {
				ythreshold = YTHRESHOLD;
				--selected;
				advance = -1;
				funprohibited = 10;
			}
		}
		else if (joystick1_x > 32) {
			if (xthreshold > 0) {
				--xthreshold;
				page_left -= (XTHRESHOLD - xthreshold);
			}
			if (xthreshold == 0 || joystick1_x > 96) {
				advance = 2;
				funprohibited = 16;
				xthreshold = XTHRESHOLD;
			} 
		}
		else if (joystick1_x < -32) {
			if (xthreshold > 0) {
				--xthreshold;
				page_left += (XTHRESHOLD - xthreshold);
			}
			if (xthreshold == 0 || joystick1_x < -96) {
				advance = -2;
				funprohibited = 16;
				xthreshold = XTHRESHOLD;
			} 
		} else {
			xthreshold = XTHRESHOLD;
			ythreshold = YTHRESHOLD;
			page_left = PAGE_LEFT;
		}

		if (advance > 0) {
			if (advance == 2 || page_start + selected == page_boundary) {
				if (advance != 2) selected = 0;
				
				if (page_boundary < total_items) {
					page_start = page_boundary;
				} else {
					page_start = 0;
				}
				page_boundary = min(page_start + ITEMS_PER_PAGE, total_items);

				flipping = FlipPageInit(1);
			}
		}

		if (advance < 0) {
			if (advance == -2 || selected == -1) {
				page_start -= ITEMS_PER_PAGE;
				if (page_start < 0) {
					page_start = ITEMS_PER_PAGE * (total_items/ITEMS_PER_PAGE);
				}
				page_boundary = min(page_start + ITEMS_PER_PAGE, total_items);

				if (advance != -2) selected = page_boundary - page_start - 1;
				
				flipping = FlipPageInit(0);
			} 
		}

		if (selected + page_start >= page_boundary) {
			selected = page_boundary - page_start - 1;
		}

		if (advance != 0) {
			animate_selected_start();
			starspeed = 16;
		}
	}

	if (flipping) {
		flipping = FlipPage(frame);
	}

	int16_t sel_y = drawPageItems();

	int16_t star_y_error = ((sel_y  * 128) - star_y) / 6;
	star_y += star_y_error;

	if (starspeed > 1) --starspeed;

	starframe += starspeed;
	Star_Animate(starframe);
	Moveto(page_left - 10 + anix, (star_y/128) - 5);
	Intensity(0x60);
	//Draw_VLp_b(starrot, 0x3f - (abs(star_y_error>>6)), 0);
	Draw_VLp_b(starrot, 0xa - (abs(star_y_error>>9)), 0);

	char debug[] = "g SVO 2014\200";
	itohex8(debug, joystick1_x);
	itohex8(debug + 3, joystick1_y);
	SetCharSizeHW(0xfc30);

	Reset0Ref();
	Print_Str_d(-36, -128, debug);

	//Reset0Ref();
	Intensity(0x7f);
	SetCharSizeHW(0xf068);
	Print_Str_d(-100, 120, "VECTREXADOR\200");

	return 0;
}

const ZoomDesc titlezoom[] = {
	{zoom:0xf850, xofs:-64, yofs:32, intensity:0x7F},
	{zoom:0xf752, xofs:-68, yofs:52, intensity:0x7F},
	{zoom:0xf654, xofs:-70, yofs:72, intensity:0x7F},
	{zoom:0xf556, xofs:-75, yofs:98, intensity:0x7F},
	{zoom:0xf458, xofs:-80, yofs:102, intensity:0x7F},
	{zoom:0xf35c, xofs:-85, yofs:108, intensity:0x7F},
	{zoom:0xf160, xofs:-90, yofs:112, intensity:0x7F},
	{zoom:0xf164, xofs:-95, yofs:118, intensity:0x7F},
	{zoom:0xf068, xofs:-100, yofs:120, intensity:0x7F},
};

int Start_Anim(int frame) {
	frame = frame/2;
	if (frame < sizeof(titlezoom)/sizeof(titlezoom[0])) {
		Intensity(0x7f);
		SetCharSizeHW(titlezoom[frame].zoom);
		Print_Str_d(titlezoom[frame].xofs, titlezoom[frame].yofs, "VECTREXADOR\200");
		return 0;
	}
	return 1;
}
                             
int main()
{
	uint8_t i = 0;
	uint8_t state = 0;

	FRAMEFUNC frame_func = MainFrame;
	MainFrameReset();
	MainFrameInit();

	enable_joystick_1x();
	enable_joystick_1y();

	frame_func = Start_Anim;

	for (uint8_t frame = 0;; frame++) {
		// wait for frame boundary (one frame = 30,000 cyles = 50 Hz)
		Wait_Recal();

		state = frame_func(frame);
		if (state) {
			frame_func = MainFrame;
		}

		// zero the integrators and set active ground
		Reset0Ref();

		Joy_Analog();
	}
	return 0;
}
