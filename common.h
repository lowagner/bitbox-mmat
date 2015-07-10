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
#define NUMBER_GAMES 3

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

extern int game, level, delayed_level; 
// level 0 : intro, level 1 : game menu, next levels : games
extern int pause;
extern LUINT score, high_score[NUMBER_GAMES];

extern uint16_t my_gamepad_buttons[2];
extern uint16_t prev_gamepad_buttons[2];

extern sij cursor;

typedef struct _game_data
{
    union {
        // memory matrix variables
        struct {
            uint8_t blocks[15][20];
            uint8_t next_blocks[15][20];
            LUINT next_best_score, best_score;
            int time_remaining, start_time, memorization_time;
            uint8_t nblocks_x, nblocks_y, next_nblocks_x, next_nblocks_y,
                memorization, next_ncol, blocks_not_prepped, max_incorrect,
                colorbucket[4];
        } mm;
        // painta
        struct {
            int8_t painting_index;
        } pp;
        // simone says:
        struct {
            uint8_t packed_blocks[600];
            int current_round, current_note, final_round, final_note, incorrect;
            uint8_t next_blocks[4], memorization;
        } ss;
    };
} game_data;

extern game_data D;


extern const int16_t gamepad_dpad;
extern const int16_t gamepad_any;


void move_sprite_extra(sij* sij, int8_t dj, int8_t di, uint8_t nblocks_x, uint8_t nblocks_y);
void move_sprite(sij* sij, int8_t dj, int8_t di);

void set_lower_block(int j, int i, uint8_t color);
void set_block(int j, int i, uint8_t color);

void swap_blocks_extra(int8_t dj1, int8_t di1, int8_t dj2, int8_t di2, uint8_t nblocks_x, uint8_t nblocks_y);
void swap_blocks(int8_t dj1, int8_t di1, int8_t dj2, int8_t di2);

void read_presses();


inline void update_score()
{
    // write the score
    int j = 3;
    int i = SCREEN_X/2+9;
    LUINT score_breakdown = score;
    while (score_breakdown)
    {
        vram[j][i--] = tmap_zero + (score_breakdown % 10);
        score_breakdown /= 10;
    }
}

#endif
