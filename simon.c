#include "simon.h"
#include "simone.h"
#include "common.h"

void set_big_block(uint8_t color)
{
    int y = SCREEN_Y/2-3;
    int x = SCREEN_X/2-1;
    switch (color)
    {
    case 0: // note right
        set_block(y, x-2, color); // upper left
        set_block(y, x, 5); // upper mid
        set_block(y, x+2, color); // upper right
        y += 2;
        set_block(y, x-2, 5); // middle left
        set_block(y, x, color); // middle mid
        set_block(y, x+2, 5); // middle right
        y += 2;
        set_block(y, x-2, color); // bottom left
        set_block(y, x, 5); // bottom mid
        set_block(y, x+2, color); // bottom right
    break;
    case 1: // note left
        set_block(y, x-2, color); // upper left
        set_block(y, x, 5); // upper mid
        set_block(y, x+2, color); // upper right
        y += 2;
        set_block(y, x-2, 5); // middle left
        set_block(y, x, 5); // middle mid
        set_block(y, x+2, 5); // middle right
        y += 2;
        set_block(y, x-2, color); // bottom left
        set_block(y, x, 5); // bottom mid
        set_block(y, x+2, color); // bottom right
    break;
    case 2: // note up
        set_block(y, x-2, color); // upper left
        set_block(y, x, color); // upper mid
        set_block(y, x+2, color); // upper right
        y += 2;
        set_block(y, x-2, color); // middle left
        set_block(y, x, color); // middle mid
        set_block(y, x+2, color); // middle right
        y += 2;
        set_block(y, x-2, color); // bottom left
        set_block(y, x, 5); // bottom mid
        set_block(y, x+2, color); // bottom right
    break;
    case 3: // note down
        set_block(y, x-2, 5); // upper left
        set_block(y, x, 5); // upper mid
        set_block(y, x+2, 5); // upper right
        y += 2;
        set_block(y, x-2, 5); // middle left
        set_block(y, x, 5); // middle mid
        set_block(y, x+2, 5); // middle right
        y += 2;
        set_block(y, x-2, 5); // bottom left
        set_block(y, x, color); // bottom mid
        set_block(y, x+2, 5); // bottom right
    break;
    default:
        set_block(y, x-2, color); // upper left
        set_block(y, x, color); // upper mid
        set_block(y, x+2, color); // upper right
        y += 2;
        set_block(y, x-2, color); // middle left
        set_block(y, x, color); // middle mid
        set_block(y, x+2, color); // middle right
        y += 2;
        set_block(y, x-2, color); // bottom left
        set_block(y, x, color); // bottom mid
        set_block(y, x+2, color); // bottom right
    }
}

void simon_game_init( void ) 
{
    simone_enter_level(START_LEVEL);
    //ply_init(SONGLEN,songdata);
}

int simon_game_frame(void) 
{
    ply_update_noloop();

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
        {
            if (delayed_level == -1)
                simone_enter_level(0);
            else
            {
                // switch from memorization to play or vice versa
                D.ss.current_round = 0;
                D.ss.current_note = 0;
                set_big_block(5); // clear the blocks

                if (D.ss.memorization)
                {
                    vram[8][16] = tmap_m;
                    vram[8][17] = tmap_e;
                    vram[8][18] = tmap_m;
                    vram[8][19] = tmap_o;
                    vram[8][20] = tmap_r;
                    vram[8][21] = tmap_i;
                    vram[8][22] = tmap_z;
                    vram[8][23] = tmap_e;
                }
                else
                {
                    decode_blocks(D.ss.current_round);

                    D.ss.incorrect = 0;
                    vram[8][16] = tmap_bg;
                    vram[8][17] = tmap_bg;
                    vram[8][18] = tmap_p;
                    vram[8][19] = tmap_l;
                    vram[8][20] = tmap_a;
                    vram[8][21] = tmap_y;
                    vram[8][22] = tmap_bg;
                    vram[8][23] = tmap_bg;
                }
            }
        }
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
        if (D.ss.memorization)
        {
            if (vga_frame % 60 == 0)
            {
                if (D.ss.current_note == 0)
                {
                    decode_blocks(D.ss.current_round);
                    uint8_t color = D.ss.next_blocks[D.ss.current_note];
                    set_big_block(color);
                    play_index(color);
                    ++D.ss.current_note;
                }
                else
                {
                    uint8_t color = D.ss.next_blocks[D.ss.current_note];
                    set_big_block(color);
                    play_index(color);
                    ++D.ss.current_note;
                    if (D.ss.current_note > 3)
                    {
                        // increment the round
                        ++D.ss.current_round;
                        D.ss.current_note = 0;
                    }
                }

                if (D.ss.current_round*4 + D.ss.current_note >= D.ss.final_note)
                {
                    D.ss.memorization = 0;
                    delayed_level = 2;
                    pause = 60;
                }
            }
        }
        else // not in memorization mode
        {
            if (PRESSED(0, dpad))
            {
                uint8_t color = D.ss.next_blocks[D.ss.current_note];
                set_big_block(color);
                if (PRESSED(0,up))
                {
                    if (color == 2)
                    {
                        score += 4*D.ss.current_round + D.ss.current_note + 1;
                        PLAY(note_up);
                        update_score();
                    }
                    else
                    {
                        PLAY(note_flub);
                        ++D.ss.incorrect;
                    }
                    ++D.ss.current_note;

                    UNPRESS(0, up);
                }
                else if (PRESSED(0, down))
                {
                    if (color == 3)
                    {
                        score += 4*D.ss.current_round + D.ss.current_note + 1;
                        PLAY(note_down);
                        update_score();
                    }
                    else
                    {
                        PLAY(note_flub);
                        ++D.ss.incorrect;
                    }
                    ++D.ss.current_note;
                    UNPRESS(0, down);
                }
                else if (PRESSED(0, right))
                {
                    if (color == 0)
                    {
                        score += 4*D.ss.current_round + D.ss.current_note + 1;
                        PLAY(note_right);
                        update_score();
                    }
                    else
                    {
                        PLAY(note_flub);
                        ++D.ss.incorrect;
                    }
                    ++D.ss.current_note;
                    UNPRESS(0, right);
                }
                else // left was pressed
                {
                    if (color == 1)
                    {
                        score += 4*D.ss.current_round + D.ss.current_note + 1;
                        PLAY(note_left);
                        update_score();
                    }
                    else
                    {
                        PLAY(note_flub);
                        ++D.ss.incorrect;
                    }
                    ++D.ss.current_note;
                    UNPRESS(0, left);
                }

                if (D.ss.incorrect >= 1 + D.ss.final_note/16)
                {
                    delayed_level = -1;
                    // write out no good.
                    vram[8][16] = tmap_n;
                    vram[8][17] = tmap_o;
                    vram[8][18] = tmap_bg;
                    vram[8][19] = tmap_g;
                    vram[8][20] = tmap_o;
                    vram[8][21] = tmap_o;
                    vram[8][22] = tmap_d;
                    vram[8][23] = tmap_period;
                    if (score > high_score[game])
                    {
                        high_score[game] = score;
                    }
                    pause = 180;
                }
                else if (D.ss.current_round*4 + D.ss.current_note >= D.ss.final_note)
                {
                    D.ss.memorization = 1;
                    if (D.ss.incorrect)
                    {
                        vram[8][16] = tmap_bg;
                        vram[8][17] = tmap_bg;
                        vram[8][18] = tmap_bg;
                        vram[8][19] = tmap_o;
                        vram[8][20] = tmap_k;
                        vram[8][21] = tmap_period;
                        vram[8][22] = tmap_bg;
                        vram[8][23] = tmap_bg;
                    }
                    else
                    {
                        vram[8][16] = tmap_p;
                        vram[8][17] = tmap_e;
                        vram[8][18] = tmap_r;
                        vram[8][19] = tmap_f;
                        vram[8][20] = tmap_e;
                        vram[8][21] = tmap_c;
                        vram[8][22] = tmap_t;
                        vram[8][23] = tmap_exclamation;
                    }
                    delayed_level = 2;
                    // we got enough right, so let's move forward:
                    ++D.ss.final_note;
                    // check if we need to increment the round, too:
                    if (D.ss.current_note > 3) //D.ss.final_note / 4 > D.ss.final_round)
                    {
                        ++D.ss.final_round;
                        pack_next_blocks(D.ss.final_round);
                    }
                    pause = 60;
                }
                else if (D.ss.current_note > 3)
                {
                    // we are finished with this round.
                    ++D.ss.current_round;
                    D.ss.current_note = 0;
                    decode_blocks(D.ss.current_round);
                }
            }
        }
    }
    return 0;
}


