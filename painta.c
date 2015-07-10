#include "painta.h"
#include "common.h"

void delete_block(int dj, int di)
{
    int i = di*2 + SCREEN_X/2 - 20; 
    int j = dj*2 + SCREEN_Y/2 - 15; 
    vram[j][i] = tmap_bg;
    vram[j][i+1] = tmap_bg;
    vram[j+1][i] = tmap_bg;
    vram[j+1][i+1] = tmap_bg;
}

void create_or_cycle_block(int dj, int di, int direction)
{
    int i = di*2 + SCREEN_X/2 - 20; 
    int j = dj*2 + SCREEN_Y/2 - 15;  
    int8_t index = vram[j][i];
    if (index < 16)
    {
        // cycle color
        index = ((index/2 + direction));
        if (index < 0)
            index = 7;
        else if (index > 3)
            index = 1;
        else
            index = 2*index+1;
        // keep track of the color we got:
        D.pp.painting_index = index;
    }
    else
        // to re-use from last time:
        index = D.pp.painting_index;
        // previously was random:
        //index = (rand()%4)*2+1;
    //message("index = %d\n", index);
    vram[j][i] = index++;
    vram[j][i+1] = index;
    index += 7;
    vram[j+1][i] = index++;
    vram[j+1][i+1] = index;
}



void painta_enter_level(int l)
{
    message(" level = %d\n", l);

    level = l;
    if (level>=REAL_LEVEL) 
    {
        //actual_array = (int*)malloc(xblocks*yblocks * sizeof(int));
        tmap_blit(bg,0,0, tmap_header, tmap_tmap[3*game+2]);
        // music player stop
        ply_init(0,0);
        move_sprite(&cursor, 7, 10);
        cursor.sprite->fr=1;
    }
    else
    {
        // copy background into vram
        tmap_blit(bg,0,0, tmap_header, tmap_tmap[3*game+level]);
        cursor.sprite->x = -16;
    }

    delayed_level = 0;
}

void painta_game_init( void ) 
{
    painta_enter_level(START_LEVEL);
    //ply_init(SONGLEN,songdata);
}

int painta_game_frame(void) 
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
                painta_enter_level(delayed_level+1);
        return 0;
    }

    if (level < REAL_LEVEL) 
    {  // men
        // just wait for keypress - wait a bit ?
        if (PRESSED(0,start))
        {
            my_gamepad_buttons[0] = 0;
            painta_enter_level(level+1);
        }
        else if (PRESSED(0, select))
        {
            my_gamepad_buttons[0] = 0;
            if (level == 0)
                return 1;
            else
                painta_enter_level(0);
            return 0;
        }
        else if (PRESSED(0,any)) 
        {
            my_gamepad_buttons[0] = 0;
            painta_enter_level(level+1);
        }
    } 
    else 
    {
        // input handling
        if (PRESSED(0,any)) 
            cursor.sprite->fr=1; 
        else
            cursor.sprite->fr=0; 

        if (PRESSED(0, start))
        {
            for (int j=0; j<SCREEN_Y; ++j)
            for (int i=0; i<SCREEN_X; ++i)
                vram[j][i] = tmap_bg;
                
            my_gamepad_buttons[0] = 0;
            return 0;
        }
        if (PRESSED(0, select))
        {
            painta_enter_level(0);
            return 0;
        }
        if (PRESSED(0, X))
        {
            create_or_cycle_block(cursor.dj, cursor.di, 1);
            UNPRESS(0, X);
        }
        else if (PRESSED(0, L))
        {
            create_or_cycle_block(cursor.dj, cursor.di, 1);
            UNPRESS(0, L);
        }
        else if (PRESSED(0, Y))
        {
            create_or_cycle_block(cursor.dj, cursor.di, -1);
            UNPRESS(0, Y);
        }
        else if (PRESSED(0, R))
        {
            create_or_cycle_block(cursor.dj, cursor.di, -1);
            UNPRESS(0, R);
        }
        else if (PRESSED(0, B))
        {
            delete_block(cursor.dj, cursor.di);
        }
        if (PRESSED(0, dpad))
        {
            if (PRESSED(0,up))
            {
                if (cursor.dj > 0)
                {
                    cursor.dj -= 1;
                    cursor.sprite->y -= 32;
                    if (PRESSED(0, A))
                        swap_blocks(cursor.dj, cursor.di, cursor.dj+1, cursor.di);
                        // don't unpress here, because we want player to be able
                        // to chain moving things around
                }
                UNPRESS(0, up);
            }
            else if (PRESSED(0, down))
            {
                if (cursor.dj < 14)
                {
                    cursor.dj += 1;
                    cursor.sprite->y += 32;
                    if (PRESSED(0, A))
                        swap_blocks(cursor.dj, cursor.di, cursor.dj-1, cursor.di);
                }
                UNPRESS(0, down);
            }
            else if (PRESSED(0, right))
            {
                if (cursor.di < 19)
                {
                    cursor.di += 1;
                    cursor.sprite->x += 32;
                    if (PRESSED(0, A))
                        swap_blocks(cursor.dj, cursor.di, cursor.dj, cursor.di-1);
                }
                UNPRESS(0, right);
            }
            else // left was pressed
            {
                if (cursor.di > 0)
                {
                    cursor.di -= 1;
                    cursor.sprite->x -= 32;
                    if (PRESSED(0, A))
                        swap_blocks(cursor.dj, cursor.di, cursor.dj, cursor.di+1);
                }
                UNPRESS(0, left);
            }
        }
    }
    return 0;
}

