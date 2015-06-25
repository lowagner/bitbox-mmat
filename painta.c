#include "painta.h"
#include "common.h"

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
        nblocks_x = 20;
        nblocks_y = 15;
        move_sprite(&cursor, nblocks_y/2, nblocks_x/2);
        cursor.sprite->fr=1;
    }
    else
    {
        // copy background into vram
        tmap_blit(bg,0,0, tmap_header, tmap_tmap[3*game+level]);
        cursor.sprite->x = -16;
        pause = 10;
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
                if (cursor.dj < nblocks_y-1)
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
                if (cursor.di < nblocks_x-1)
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

