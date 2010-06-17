#include <tonc_gba.h>
#include "lifemaps.h"
#include "life.h"
#include "lamerand.h"

#define LIFEMAP_SIZE 32

static u8 lifemap_orig[LIFEMAP_SIZE*LIFEMAP_SIZE];
static u8 lifemap_trans[LIFEMAP_SIZE*LIFEMAP_SIZE];

static u8* lifemap_out;
static u8* lifemap_in;

void init_lifemap() {
	memset16(lifemap_orig, 0, LIFEMAP_SIZE*LIFEMAP_SIZE/2);

	lifemap_in = lifemap_orig;
	lifemap_out = lifemap_trans;
}


void life_newmap(u16 *index) {
	dma_memcpy(lifemap_orig, life_get_map(*index), LIFEMAP_SIZE*LIFEMAP_SIZE/4);
	*index = (*index+1) % lifemaps_length;

	lifemap_in = lifemap_orig;
	lifemap_out = lifemap_trans;
}

void life_setmap(u16 index) {
	u16 i = index;

	life_newmap(&i);
}


// if offset is 16-bit nasty things happen, can't figure what

static inline u16 count_neighbours_super(u32 offset) {
	u8* rowptr = lifemap_in + offset - (LIFEMAP_SIZE + 1);
	int n = 0;

	n += *(rowptr++); n += *(rowptr++); n += *(rowptr++);
	rowptr += LIFEMAP_SIZE-3;
	n += *(rowptr++); rowptr++; n += *(rowptr++);
	rowptr += LIFEMAP_SIZE-3;
	n += *(rowptr++); n += *(rowptr++); n += *(rowptr++);

	return n;	
}



int life_cycle_multistate(int reset) {
	static u16 state = 0;

	u16 x, ii;
	static u16 y;  	// y spans across multiple invocations

	static u8* current_cell;
	static u32 source_offset;
	u8* temp;

	u16 needupdate = 0;

	if (reset) {
		state = 0;
		return needupdate;
	}

	switch (state) {
		case 0:
			// state 0: prepare
			dma_memcpy(lifemap_out, lifemap_in, LIFEMAP_SIZE*LIFEMAP_SIZE/4);
			current_cell = lifemap_out;
			state = 1;
			//break;
		case 1:
			// state 1: zero out first line
        	for (x = 0; x < LIFEMAP_SIZE; x++) {
        		*(current_cell++) = 0;
        	}
        	y = 1;
        	source_offset = LIFEMAP_SIZE;			// prep offset to 1st line (not 0th)
        	state = 2;
        	break;
        case 2:
        	// state 2: process first half of the field
        	for (ii = 0; ii < lines_at_a_time && y < LIFEMAP_SIZE-1; ii++, y++) {
        		*(current_cell++) = 0;
        		source_offset++;
        		for (x = 1; x < LIFEMAP_SIZE-1; x++) {
        			switch (count_neighbours_super(source_offset)) {
        				case 2:
        					// just skip
        					//*current_cell = lifemap_in[(y<<5)+x];
        					break;
        				case 3:
        					*current_cell = 1;
        					break;
        				default:
        					*current_cell = 0;					
        					break;
        			}
        			current_cell++;
                    source_offset++;
        		}
        		*current_cell++ = 0;
        		source_offset++;
        	}
        	if (y == LIFEMAP_SIZE-1) {
        		state = 3;
        	}
        	break;
		case 3:
			// state 4: zero out last line
        	for (x = 0; x < LIFEMAP_SIZE; x++) {
        		*(current_cell++) = 0;
        	}
        	state = 4;
        	break;
        case 4:
        	// state 5: swap in/out
        	temp = lifemap_in;
        	lifemap_in = lifemap_out;
        	lifemap_out = temp;
        	needupdate = 1;
        	state = 0;
        	break;
	}

	return needupdate;
}
                                     
void life_copy_to_bg(BGINFO* bg) {
	dma_memcpy(bg->map, lifemap_in, LIFEMAP_SIZE*LIFEMAP_SIZE/4);
}
