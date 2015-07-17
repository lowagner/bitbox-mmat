#ifndef SIMONE_H
#define SIMONE_H
#include "stdint.h"

int simone_game_frame(void);
void simone_game_init(void);

void simone_enter_level(int l);

void pack_next_blocks(int round);
void decode_blocks(int round);
void play_index(uint8_t color_index);

#endif



