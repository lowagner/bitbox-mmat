#include "common.h"
#include "memmat.h"
#include "sandbox.h"
#include <math.h> // sqrtf

uint8_t vram[SCREEN_Y][SCREEN_X];
//extern char build_sprite_spr[];
//extern const unsigned char songdata[];
//extern const unsigned char ticktockdata[];

object *bg;

// game state
// ----
frame_fn* the_game_frame;

int game, level, delayed_level; // level 0 : intro, level 1 : game menu, next levels : games
int time_remaining, pause, start_time;
sij cursor;// player cursor position in the matrix.
uint8_t memorization; // time to memorize or play
uint8_t nblocks_x, nblocks_y, next_nblocks_x, next_nblocks_y;
int memorization_time; // # of seconds for memorization
LUINT score, best_score, next_best_score, high_score[3]; // best score for the layout
uint8_t blocks_not_prepped; // ready for next layout or not
uint8_t next_ncol;
uint8_t colorbucket[4];
uint8_t max_incorrect; // whether there can be more than one distinguishable switch possible
uint16_t my_gamepad_buttons[2];
uint16_t prev_gamepad_buttons[2];

uint8_t blocks[15][20];
uint8_t next_blocks[15][20];


// constants
// ----

#define NUMBER_GAMES 2

const int16_t gamepad_dpad = 15 << 8;
const int16_t gamepad_any = 63;

// code
// ----


void enter_game(int g)
{
    game = g%NUMBER_GAMES; 
    message("opening game %d:\n", game);
    switch (game)
    {
    case 0:
        memmat_game_init();
        the_game_frame = &memmat_game_frame;
        break;
    case 1:
        sandbox_game_init();
        the_game_frame = &sandbox_game_frame;
        break;
    default:
        memmat_game_init();
        the_game_frame = &memmat_game_frame;
    }
}

void game_init( void ) 
{
    for (uint8_t i=0; i<3; ++i)
        high_score[i] = 0;
    // reset buttons to zero
    for (uint8_t i=0; i<2; ++i)
        my_gamepad_buttons[i] = 0;
    for (uint8_t i=0; i<2; ++i)
        prev_gamepad_buttons[i] = 0;
    
    blitter_init(); 
    bg = tilemap_new(tmap_tset,0,0,TMAP_HEADER(SCREEN_X,SCREEN_Y,TSET_16, TMAP_U8), vram); 
    cursor.sprite = sprite_new(build_sprite_spr,0,0,0);

    ply_init(SONGLEN,songdata);
    enter_game(0);
}

void game_frame( void ) 
{
    if (the_game_frame)
        if (the_game_frame())
            enter_game(game+1);
}
