#ifndef COMMON_H
#define COMMON_H

#include <bitbox.h>
#include <blitter.h>
#include <chiptune_player.h>

#include "build/tmap.h"
#include <stdlib.h> // srand rand

#include "song.h"
#include "ticktock.h"

#define START_LEVEL 0
#define REAL_LEVEL 2

#define UNPRESS(id, key) my_gamepad_buttons[id] -= gamepad_##key
#define PRESSED(id, key) my_gamepad_buttons[id] & gamepad_##key
#define PRESS(id, key) my_gamepad_buttons[id] |= gamepad_##key
#define PREV_PRESSED(id, key) prev_gamepad_buttons[id] & gamepad_##key

#define SCREEN_X 40
#define SCREEN_Y 30

#define UINT unsigned int
#define LUINT unsigned long long int

typedef int (frame_fn)(void);

extern frame_fn* the_game_frame;

extern object *bg;

typedef struct _sij
{
    int8_t di, dj;
    object* sprite;
} sij;

extern uint8_t vram[SCREEN_Y][SCREEN_X];
extern char build_sprite_spr[];

extern const unsigned char songdata[];
extern const unsigned char ticktockdata[];

extern int game, level, delayed_level; // level 0 : intro, level 1 : game menu, next levels : games
extern int time_remaining, pause, start_time;
extern sij cursor;// player cursor position in the matrix.
extern uint8_t memorization; // time to memorize or play
extern uint8_t nblocks_x, nblocks_y, next_nblocks_x, next_nblocks_y;
extern int memorization_time; // # of seconds for memorization
extern LUINT score, best_score, next_best_score, high_score[3]; // best score for the layout
extern uint8_t blocks_not_prepped; // ready for next layout or not
extern uint8_t next_ncol;
extern uint8_t colorbucket[4];
extern uint8_t max_incorrect; // whether there can be more than one distinguishable switch possible
extern uint16_t my_gamepad_buttons[2];
extern uint16_t prev_gamepad_buttons[2];

extern const int16_t gamepad_dpad;
extern const int16_t gamepad_any;

extern uint8_t blocks[15][20];
extern uint8_t next_blocks[15][20];

void move_sprite(sij* sij, int8_t dj, int8_t di);

void set_lower_block(int j, int i, uint8_t color);

void create_or_cycle_block(int dj, int di, int direction);
void delete_block(int j, int i);
void set_block(int j, int i, uint8_t color);

void swap_blocks(int8_t dj1, int8_t di1, int8_t dj2, int8_t di2);

void read_presses();


#endif
