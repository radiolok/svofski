#ifndef _LIFE_H
#define _LIFE_H

#define lines_at_a_time 32


void init_lifemap();
void life_newmap(u16 *index);
void life_setmap(u16 index);
int life_cycle_multistate(int reset);
void life_copy_to_bg(BGINFO* bg);



#endif
