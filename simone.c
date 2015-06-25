#include "simone.h"
#include "common.h"

// reuse a few variables from mmat that we don't use:

#define current_round memorization_time
#define final_round start_time
#define current_note blocks_not_prepped 
#define incorrect next_ncol


void get_next_blocks(int round)
{
    for (uint8_t i=0; i<4; ++i)
        colorbucket[i] = rand()%4; // must be in the range 0 to 3

    // put the result into the first four values of next_blocks
    blocks[round/nblocks_x][round%nblocks_x] = (
        (colorbucket[0]<<6) |
        (colorbucket[1]<<4) | 
        (colorbucket[2]<<2) |
        (colorbucket[3]<<0)
    );
}

void decode_blocks(int round)
{
    uint8_t block = blocks[round/nblocks_x][round%nblocks_x];
    colorbucket[0] = (block>>6) & 3;
    colorbucket[1] = (block>>4) & 3;
    colorbucket[2] = (block>>2) & 3;
    colorbucket[3] = (block>>0) & 3;
}


void simone_enter_level(int l)
{
    message(" level = %d\n", l);
    cursor.sprite->x = -16;

    level = l;
    if (level>=REAL_LEVEL) 
    {
        //actual_array = (int*)malloc(xblocks*yblocks * sizeof(int));
        tmap_blit(bg,0,0, tmap_header, tmap_tmap[3*game+2]);
        // music player stop
        ply_init(0,0);
        nblocks_x = 20;
        nblocks_y = 15;

        memorization = 1;
        current_round = 0;
    }
    else
    {
        // copy background into vram
        tmap_blit(bg,0,0, tmap_header, tmap_tmap[3*game+level]);
        pause = 10;
        if (level == 0)
        {
            // initialize everything
            final_round = 0;  // how many rounds have we gotten up to
            current_note = 0;
            get_next_blocks(0); // get the first set of blocks into "colorbucket"
        }
    }

    delayed_level = 0;
}

void simone_game_init( void ) 
{
    simone_enter_level(START_LEVEL);
    //ply_init(SONGLEN,songdata);
}

int simone_game_frame(void) 
{
    ply_update();

    read_presses();

    if (pause)
    {
        pause--;
        if (PRESSED(0,start) && pause > 5) 
        {
            pause = 2;
            UNPRESS(0, start);
        }
        if (!pause)
            if (delayed_level)
                simone_enter_level(delayed_level+1);
        return 0;
    }

    if (level < REAL_LEVEL) 
    {  
        // just wait for keypress - wait a bit ?
        if (PRESSED(0,start))
        {
            my_gamepad_buttons[0] = 0;
            simone_enter_level(level+1);
        }
        else if (PRESSED(0, select))
        {
            my_gamepad_buttons[0] = 0;
            if (level == 0)
                return 1;
            else
                simone_enter_level(0);
            return 0;
        }
        else if (PRESSED(0,any)) 
        {
            my_gamepad_buttons[0] = 0;
            simone_enter_level(level+1);
        }
    } 
    else 
    {
        if (memorization)
        {
            if (vga_frame % 60 == 0)
            {
                if (current_note < 4)
                {
                    set_block(SCREEN_Y/2-1, SCREEN_X/2-4 + 2*current_note, colorbucket[current_note]);
                    ++current_note;
                }
                else
                {
                    ++current_round;
                    current_note = 0;
                    if (current_round > final_round)
                    {
                        memorization = 0;
                        current_round = 0;

                        vram[10][14] = tmap_p;
                        vram[10][15] = tmap_l;
                        vram[10][16] = tmap_a;
                        vram[10][17] = tmap_y;
                    }
                    else
                        decode_blocks(current_round);
                    for (uint8_t i=0; i<4; i++)
                        set_block(SCREEN_Y/2-1, SCREEN_X/2-4 + 2*i, 5);
                }
            }
        }
        else // not in memorization mode
        {
            if (PRESSED(0, dpad))
            {
                set_block(SCREEN_Y/2-1, SCREEN_X/2-4 + 2*current_note, colorbucket[current_note]);
                if (PRESSED(0,up))
                {
                    if (colorbucket[current_note] == 2)
                    {
                        // play nice note?
                    }
                    else
                    {
                        // play bad note?
                        ++incorrect;
                    }
                    ++current_note;

                    UNPRESS(0, up);
                }
                else if (PRESSED(0, down))
                {
                    if (colorbucket[current_note] == 3)
                    {
                        // play nice note?
                    }
                    else
                    {
                        // play bad note?
                        ++incorrect;
                    }
                    ++current_note;
                    UNPRESS(0, down);
                }
                else if (PRESSED(0, right))
                {
                    if (colorbucket[current_note] == 0)
                    {
                        // play nice note?
                    }
                    else
                    {
                        // play bad note?
                        ++incorrect;
                    }
                    ++current_note;
                    UNPRESS(0, right);
                }
                else // left was pressed
                {
                    if (colorbucket[current_note] == 1)
                    {
                        // play nice note?
                    }
                    else
                    {
                        // play bad note?
                        ++incorrect;
                    }
                    ++current_note;
                    UNPRESS(0, left);
                }

                if (current_note > 3)
                {
                    // we are finished with this round.
                    ++current_round;
                    current_note = 0;
                    if (current_round > final_round)
                    {
                        memorization = 1;
                        current_round = 0;
                        ++final_round;
                        get_next_blocks(final_round);
                        
                        vram[10][10] = tmap_m;
                        vram[10][11] = tmap_e;
                        vram[10][12] = tmap_m;
                        vram[10][13] = tmap_o;
                        vram[10][14] = tmap_r;
                        vram[10][15] = tmap_i;
                        vram[10][16] = tmap_z;
                        vram[10][17] = tmap_e;
                    }
                    decode_blocks(current_round);
                    for (uint8_t i=0; i<4; i++)
                        set_block(SCREEN_Y/2-1, SCREEN_X/2-4 + 2*i, 5);
                }
            }
        }
    }
    return 0;
}


