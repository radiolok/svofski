#include <stdio.h>
#include <stdlib.h>

#include <inttypes.h>

#include "vectrex.h"
#include "pecado.h"


#define ITEMS_PER_PAGE	10

typedef int (*FRAMEFUNC)(int);

const int8_t boxpath[] = {
   0,   -10,-10,    
   255,  10,-10,    
   255,  10, 10,
   255, -10, 10,
   255, -10,-10, 
   1};

const int8_t starpath[] = {
	0, 9, 3,
	255, -6,-8,
	255,  0,10,
	255,  6,-8,
	255,  -9, 3,
	255,  9, 3,
	1};

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
	"TITLE14\200",
	"TITLE15\200",
	"TITLE16\200",
	"TITLE17\200",
	"TITLE18\200",
	"TITLE19\200",
	"TITLE20\200",
};

typedef struct zoomani_ {
	uint16_t zoom;
	int8_t xofs;
	int8_t yofs;
	uint8_t intensity;
} ZoomDesc;

const ZoomDesc selectzoom[] = {
	{zoom:0xf850, xofs:0, yofs:0, intensity:0x20},
	{zoom:0xf851, xofs:0, yofs:0, intensity:0x42},
	{zoom:0xf852, xofs:0, yofs:0, intensity:0x58},
	{zoom:0xf753, xofs:-1, yofs:0, intensity:0x6c},
	{zoom:0xf754, xofs:-1, yofs:1, intensity:0x70},
	{zoom:0xf656, xofs:-2, yofs:1, intensity:0x78},
	{zoom:0xf558, xofs:-2, yofs:1, intensity:0x7f},
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

void animate_star(uint8_t frame) {
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

#define min(a,b) ((a)<(b)?(a):(b))

int8_t starspeed;
int16_t star_y;
uint8_t starframe;
uint8_t selected;
uint8_t page_start;
uint8_t total_items;
uint8_t page_boundary;

void MainFrame_Init() {
	starspeed = 0;
	total_items = sizeof(names)/sizeof(names[0]);
	page_boundary = min(ITEMS_PER_PAGE, sizeof(names)/sizeof(names[0]));
}

int MainFrame(int frame) {
	if ((frame & 0x1f) == 0) {
		selected++;
		if (page_start + selected == page_boundary) {
			selected = 0;
			
			if (page_boundary < total_items) {
				page_start = page_boundary;
			} else {
				page_start = 0;
			}
			page_boundary = min(page_start + ITEMS_PER_PAGE, total_items);
		}

		animate_selected_start();
		starspeed = 16;
	}

	char debug[] = "##:##:##\200";
	itohex8(debug, page_start);
	itohex8(debug + 3, page_boundary);
	//itohex8(debug + 6, buttfuck); //min(page_start + ITEMS_PER_PAGE, total_items));
	SetCharSizeHW(0xfc30);
	Print_Str_d(-120, 0, debug);

	int16_t sel_y;

	for (int8_t i = page_start, y = 120; i < page_boundary; i++, y -= 256/ITEMS_PER_PAGE) {
		char *title = names[i];
		if (i - page_start == selected) {
			animate_selected();

			Print_Str_d(-20 + anix, y + aniy, title);
			Print_Str_d(-20 + anix, y + 1 + aniy, title);
			sel_y = y;
		} else {
			Intensity(0x60);
			SetCharSizeHW(0xf850);
			Print_Str_d(-20, y, title);
		}
	}

	int16_t star_y_error = ((sel_y * 128) - star_y) / 6;
	star_y += star_y_error;

	if (starspeed > 1) --starspeed;

	starframe += starspeed;
	animate_star(starframe);
	Moveto(-30 + anix, (star_y/128) - 5);
	Intensity(0x48);
	Draw_VLp_b(starrot, 0x7f - (abs(star_y_error>>6)), 0);

	return 0;
}
                             
int main()
{
	uint8_t i = 0;

	FRAMEFUNC frame_func = MainFrame;
	MainFrame_Init();

	for (uint8_t frame = 0;; frame++) {
		// wait for frame boundary (one frame = 30,000 cyles = 50 Hz)
		Wait_Recal();

		frame_func(frame);

		// zero the integrators and set active ground
		Reset0Ref();
	}
	return 0;
}
