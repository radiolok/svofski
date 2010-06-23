#define FRAS
//#define BOYSCOUT
#define MAD

#ifdef USE_STDLIB
#include <stdlib.h>
#endif

#include <tonc_gba.h>
#include <luts.h>

#ifdef BOYSCOUT
#include <boyscout.h>
#include "songs/ladybug.h"
#endif

#ifdef FRAS
#include "mods/pkunk.h"
#include "fras/fras.h"
#endif

#include "bg.h"
#include "wtf_map.h"
#include "bitmaps/hare.h"
#include "bitmaps/map_tiles.h"
#include "bitmaps/omg.h"
#include "text.h"
#include "lifemaps.h"

#include "lamerand.h"
#include "life.h"
#include "messages.h"

//MULTIBOOT



#define TB 0
#define MAP 8
#define MAP_AFF_SIZE	0x80

BGINFO bg2;
BGINFO bg3;


OBJINFO *oi_face = &oi_buffer[0];
s32	oi_face_rotation = 0;

u16 GLOBAL_FLAG_shutdown = 0;
u16 GLOBAL_FLAG_die = 0;

#ifdef FRAS
extern volatile int fras_currentPatternLine;
#endif

void rotate_hare(int, int);

void init_hare() {
	oam_init();

	dma_memcpy(&tl_mem[TB_OBJ][0], hareData, hareLen/4);


	oi_set_attr(oi_face, OI_A0_AFF_FLAG | OI_A0_PAL256 | OI_A0_SIZE2X,	// Square Aff-sprite
		OI_A1_SIZE_64 | oi_aff2oam(0),			// 64x64, using obj_aff[0]
		0);									// pal256, tile 0

	oi_set_pos(oi_face, 120-hareWidth, 80-hareHeight);
   	oa_identity(&oa_buffer[0]);
	dma_memcpy(pal_obj_mem, harePal, harePalLen/4);
	rotate_hare(0, 0x1000);
	oam_update(0, 3);
}

void rotate_hare(int angle, int scale) {
	OBJAFF *oa_curr= &oa_buffer[0];
	OBJAFF *oa_new=  &oa_buffer[2];
	
	oa_identity(oa_curr);
	oa_identity(oa_new);

	oa_scale(oa_curr, scale, scale);
	oa_rotate(oa_new, angle & SIN_MASK);
	oa_postmultiply(oa_curr, oa_new);

   	oi_update(0, 1);
   	oa_update(0, 3);
}

// init backrounds - if palette == 1 init only palette - do nothing else, otherwise zero out palette
void init_flowers_bg3(u16 palette) {
	int ii, loop;
	u16 *moo;
	u16 *srcmoo;


	if (palette) {
      	for (ii = 0; ii < 256; ii++) {
      		pal_bg_mem[ii] = map_tilesPal[ii];
      	}
    } else {
    	memset16(pal_bg_mem, 0, 256);

        // life background
    	bg_init(&bg2, 3, TB, MAP, BG_AFF_256 | BG_AFF_WRAP);
    	bg_set_pos(&bg2, 0, 0);

    	moo = (u16*)tl_mem[TB];
    	srcmoo = (u16*)map_tilesData;
    	for(loop = 0; loop < map_tilesLen/2; loop++) {  //load tile image data
    		moo[loop] = srcmoo[loop];
    	}


    	// bg3
    	bg_init(&bg3, 2, TB, 16, BG_AFF_256 | BG_AFF_WRAP);
    	bg_set_pos(&bg3, 0, 0);

    	srcmoo = (u16*)wtf_map;
    	for (ii = 0; ii < 32*32/2; ii++) {
    		bg3.map[ii] = srcmoo[ii];
    	}
    }
}



void shake_chars() {
	int i;
	int yo;
	int xo;

	for (i = 0; i < 64; i++) {
        yo = 1 - lamerand();
        xo = 1 - lamerand();
		text_move_rel(i, xo, yo);						
		text_clamp_position(i);
	}
	oam_update(16,64);
}

#define STAGE_FLOWERBLEND_START 	110
#define STAGE_FLOWERBLEND_L1	    16

// blend in the flowers 
void stage1_blend_in(int frame) {
	const int step1 = 1;
	static int blend1 = 0;
	static u32 p0 = 0, p1 = 0, p2 = 0;
	static u32 s0 = 25*256/STAGE_FLOWERBLEND_START, s1=25*256/STAGE_FLOWERBLEND_START, s2 = 31*256/STAGE_FLOWERBLEND_START;

	if (frame > STAGE_FLOWERBLEND_START+STAGE_FLOWERBLEND_L1*2) {
		return;
	}
	if (frame < STAGE_FLOWERBLEND_START) {
		pal_bg_mem[0x99] = RGB15(p0>>8, p1>>8, p2>>8);
		p0 += s0;
		p1 += s1;
		p2 += s2;
		
	}
	if (frame >= STAGE_FLOWERBLEND_START && frame < STAGE_FLOWERBLEND_START+STAGE_FLOWERBLEND_L1) {
		pal_bg_mem[0x99] = RGB15(25, 25, 31);//(25 << REDSHIFT) | (25 << GREENSHIFT) | (31>>BLUESHIFT);
    	blend1 += step1;
		bld_set_weights(blend1>>3, 0x80>>3);
	}
	if (frame == STAGE_FLOWERBLEND_START+STAGE_FLOWERBLEND_L1) {
		blend1 = 96;
	}
	if (frame > STAGE_FLOWERBLEND_START+STAGE_FLOWERBLEND_L1 && frame < STAGE_FLOWERBLEND_START+STAGE_FLOWERBLEND_L1*2) {
		blend1 -= 0x4;
		bld_set_weights(0x60>>3, blend1>>3);	
	}
	if (frame == STAGE_FLOWERBLEND_START+STAGE_FLOWERBLEND_L1*2) {
		bld_set_weights(0x60>>3, 0x38>>3);
	}
}


void stageX_fadeout(int frame) {
	static u16 not_finished = 1;
	static s32 p0 = 25<<8, p1 = 25<<8, p2 = 31<<8;
	static u32 s0 = 25*256/STAGE_FLOWERBLEND_START, s1=25*256/STAGE_FLOWERBLEND_START, s2 = 31*256/STAGE_FLOWERBLEND_START;

	static u16 blend_end1 = 0x0;
	static u16 blend_end2 = 0x80;
	static u16 blend_step1 = 0x2;
	static u16 blend_step2 = 0x1;

	static u16 blend_1 = 0x60;
	static u16 blend_2 = 0x38;

	if (GLOBAL_FLAG_shutdown && not_finished) {
		pal_bg_mem[0x99] = RGB15(p0>>8, p1>>8, p2>>8);
		p0 -= s0;
		p1 -= s1;
		p2 -= s2;

		if (p0 <= 0) {
			not_finished = 0;
		}

		if (blend_1 > blend_end1) {
			blend_1 -= blend_step1;
		}
		if (blend_2 < blend_end2) {
			blend_2 += blend_step2;
		}
		bld_set_weights(blend_1>>3, blend_2>>3);
	}

	if (!not_finished) {
#ifdef FRAS
		if (fras_currentPatternLine == 0) {
			FrasPauseMod();
            // avoid weird effects on cycle counter overflow - just get real dead!
            if (GLOBAL_FLAG_die) {
            	while (1);
            }
		}
#endif
	}
}


int hare_rotdir = 3;
int hare_rotmax = 45;
int hare_rotangle = 0;

int hare_scale = 2 << 8;
int hare_scale_dir = 32;
int hare_scale_max = 4 << 8;
int hare_scale_min = 128;

void stage1_zoom_in_hare(int frame) {
	if (frame > 128) {
		return;
	}

	if (frame < 2) {
		hare_scale = hare_scale_max*2;
	} else if (frame > 2 && frame < 128) {
		hare_scale -= 15;
	} else if (frame == 128) {
		hare_scale = hare_scale_min;
	}
}

void stageX_zoom_out_hare(int frame) {
	if (GLOBAL_FLAG_shutdown && hare_scale < hare_scale_max<<4) {
		hare_scale += 25;
	}
}

#define STAGE_SPIN_START 	80
#define STAGE_SPIN_LENGTH 	512
void stage1_1_spin_hare(int frame) {
	static int rotadd = 1;

	if (frame == STAGE_SPIN_START+STAGE_SPIN_LENGTH) {
		hare_rotangle = 0;
	}
	if (frame < STAGE_SPIN_START || frame > STAGE_SPIN_START+STAGE_SPIN_LENGTH) {
		return;
	}

	hare_rotangle += rotadd;
	hare_rotangle &= SIN_MASK;

	rotadd = (frame - STAGE_SPIN_START) / 32;
}

void stage2_bounce_hare(int frame) {
	if (GLOBAL_FLAG_shutdown) {
		//hare_rotangle = 0;
		//hare_scale = hare_scale_max*16;
	} else {
		if (frame > STAGE_SPIN_START+STAGE_SPIN_LENGTH) {
         	// do the hare
         	hare_rotangle += hare_rotdir;
         	if (hare_rotangle > hare_rotmax || hare_rotangle < -hare_rotmax) {
         		hare_rotdir = -hare_rotdir;
         	}

         	hare_scale += hare_scale_dir;
         	if (hare_scale < hare_scale_min || hare_scale > hare_scale_max) {
         		hare_scale_dir = -hare_scale_dir;
         	}
        }
  	}
}

#define LIFE_START	512
#define DX			256

#define LIFE_SCALEDIR	2

void stage3_life_life(int frame, FIXED *bgscale, AFF_SRC_EX *asx) {
	static u16 current_lifemap = 0;
	static u16 life_hold = 128;
	static u16 life_needupdate = 0;
	static u16 end_of_life = 0;

	static u16 waiting = 0;

	if (frame < LIFE_START && waiting) {
		return;
	}

	if (frame == LIFE_START) {
   		life_newmap(&current_lifemap);
	}

   	if ( life_hold == 128 ) {
   		if (frame >= LIFE_START) {
			if (GLOBAL_FLAG_shutdown) {
				life_setmap(4);
				end_of_life = 1;
				bg_init(&bg2, 3, TB, MAP, BG_AFF_256);	// reset wrap
				REG_DISPCNT = VID_MODE2 | VID_BG3 | VID_OBJ | VID_LINEAR; 	// disable bg3
			} else {
	   			life_newmap(&current_lifemap);
	   		}
   		}
   		if (!end_of_life) {
	   		*bgscale = 0x50;
   		}
   		life_cycle_multistate(1);
   		life_hold = 0;
   		asx->alpha = 460;
   		waiting = 1;
   	} else {
   		if (life_hold++ < 20) {
   			if (waiting) {
   				life_copy_to_bg(&bg2);
				waiting = 0;
   			}
   		} else {
   			if (!end_of_life) {
#ifdef MAD
           		*bgscale+=DX*LIFE_SCALEDIR/128;
#endif
      			if (life_needupdate) {
          			life_copy_to_bg(&bg2);
          			life_needupdate = 0;
      			} else {
    	       		life_needupdate = life_cycle_multistate(0);
               	}
           	}
   		}
   	}
   	
  	// rotate life map (if not shutting down)
  	if (!end_of_life) {
#ifdef MAD
  		asx->alpha+=2;
	  	asx->alpha = asx->alpha % SIN_SIZE;
#endif
  	} else {
  		if (*bgscale > 0x75) {
  			*bgscale+=-DX*LIFE_SCALEDIR/128;
  		} else {
  			GLOBAL_FLAG_die = 1;
  		}
  	}
	asx->sx = asx->sy = (1<<16)/(*bgscale);
}   


void stage3_text(int frame) {
	static int current_text_idx = 0;
	static int one_countdown = 0;
	static int two_countdown = 0;

	if (current_text_idx == -1) {
		GLOBAL_FLAG_shutdown = 1;
		return;
	}

	if (frame == text_strings[current_text_idx].start_frame) {
		TextString *current = &text_strings[current_text_idx];
		if (current_text_idx % 2 == 0 && one_countdown == 0) {
			text_display_string_one(current->text, current->x, current->y);
			one_countdown = current->display_time;
		} else {
			text_display_string_two(current->text, current->x, current->y);
			two_countdown = current->display_time;
		}
		current_text_idx++;
		if (text_strings[current_text_idx].text == 0) {
			current_text_idx = -1;
		}
	}
	if (one_countdown > 0) {
		if (--one_countdown == 0) {
			text_display_string_one("", 0, 0);
		}
	}
	if (two_countdown > 0) {
		if (--two_countdown == 0) {
			text_display_string_two("", 0, 0);
		}
	}
}

void do_stuff() {
	AFF_SRC_EX asx = {0, 0, 0, 0, 0x0100, 0x0100, 0};
	AFF_SRC_EX asx_flowers = {0, 0, 0, 0, 0x0100, 0x0100, 0};
	const int FLOWERS_DX = 256;
	FIXED ss = 0x050;

#ifndef MAD
	ss = 0x40;
#endif	

	u16 cycle = 0;

	asx.px = 128*FLOWERS_DX;
	asx.py = 128*FLOWERS_DX;
	asx.qx = 120;
	asx.qy = 80;

	int flowers_xpos = 0;
	int flowers_dxpos = 1;

	while (1) {
		stage1_blend_in(cycle);
		stage1_1_spin_hare(cycle);
		stage1_zoom_in_hare(cycle);
		stage3_text(cycle);
		shake_chars(cycle);

		stage3_life_life(cycle, &ss, &asx);

		if (GLOBAL_FLAG_shutdown) {
			stageX_fadeout(cycle);
			stageX_zoom_out_hare(cycle);
		}

		bg_rotscale(&bg2, &asx);

		stage2_bounce_hare(cycle);

		asx_flowers.px = 128*FLOWERS_DX;
		asx_flowers.py = 128*FLOWERS_DX;
		asx_flowers.qx = flowers_xpos / 4;
		asx_flowers.qy = 80;
		asx_flowers.sx = asx_flowers.sy = 0x100;

		bg_rotscale(&bg3, &asx_flowers);
		flowers_xpos = (flowers_xpos + flowers_dxpos) % (256*4);// % 4096;

		vid_vsync();
#ifdef BOYSCOUT
		BoyScoutUpdateSong();
#endif
		bg_update(&bg3);
		bg_update(&bg2);
        rotate_hare(hare_rotangle, hare_scale);

		cycle++;
	}
}

#ifdef BOYSCOUT
void init_boyscout() {
	u32 songSize;

	BoyScoutInitialize();

	songSize = BoyScoutGetNeededSongMemory((unsigned char*)SongData_Ladybug);
	BoyScoutSetMemoryArea((unsigned int)malloc(songSize));
	BoyScoutOpenSong((unsigned char *)SongData_Ladybug);

	BoyScoutPlaySong(1);
}
#endif

#ifdef FRAS

void FrasUpdateMixer(void);
void FrasTimer1Intr(void);

void init_fras() {
	int_init();
	IntrTable[2] = FrasUpdateMixer;
	IntrTable[4] = FrasTimer1Intr;

	int_enable_ex(II_VCOUNT, FrasUpdateMixer);
	int_enable_ex(II_TM1, FrasTimer1Intr);

	FrasInstall(FRAS_MEDIUM_FRQ);
	FrasPlayMod(&pkunkModInfo);
}
#endif

void init_music() {
#ifdef BOYSCOUT
	init_boyscout();
#endif
#ifdef FRAS
	init_fras();
#endif
}

void AgbMain() {
	REG_DISPCNT = VID_MODE2 | VID_BG2 | VID_BG3 | VID_OBJ | VID_LINEAR;
	bld_cnt(BLD_BG2, BLD_BG3, BLD_STD);
	bld_set_weights(0x0>>3, 0x80>>3);

	lame_randomize();

	// load backgrounds and zero out palette
	init_flowers_bg3(0);
	init_lifemap();
	life_copy_to_bg(&bg2);
	bg_update(&bg3);
	bg_update(&bg2);

	init_hare();
	init_music();
	font_load8x8();

	vid_vsync();

	// load backgrounds palette
	init_flowers_bg3(1);

    // blank out annoying magenta - this is important for the last scene with non-wrapped background
	pal_bg_mem[0x00] = 0;

	do_stuff();
}

int main() {
    AgbMain();
    return 0;
}
