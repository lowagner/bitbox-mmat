#include "common.h"
#include "memmat.h"
#include "painta.h"
#include "simone.h"
#include <math.h> // sqrtf


uint8_t vram[SCREEN_Y][SCREEN_X];
//extern char build_sprite_spr[];
//extern const unsigned char songdata[];
//extern const unsigned char ticktockdata[];

object *bg;
sij cursor;

// game state
// ----
frame_fn* the_game_frame;

game_data D;
int game, level, delayed_level; 
// level 0 : intro, level 1 : game menu, next levels : games
int pause;
LUINT score, high_score[NUMBER_GAMES];

uint16_t my_gamepad_buttons[2];
uint16_t prev_gamepad_buttons[2];

sij cursor;


// constants
// ----

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
        painta_game_init();
        the_game_frame = &painta_game_frame;
        break;
    case 2:
        simone_game_init();
        the_game_frame = &simone_game_frame;
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
