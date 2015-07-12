#include "simone.h"
#include "common.h"

inline void update_round()
{
    int j = 25;
    int i = 35;
    int round = D.ss.current_round;
    while (i > 31)
    {
        vram[j][i--] = tmap_zero + (round % 10);
        round /= 10;
    }
}

void pack_next_blocks(int round)
{
    // re-seed the random number generator every once in a while,
    // so that the player can't get spiffy:
    if (round % 16 == 0)
        srand(vga_frame);

    for (int i=0; i<4; ++i)
        D.ss.next_blocks[i] = rand()%4; // must be in the range 0 to 3
    
    message("got blocks %d, %d, %d, %d for round %d\n", D.ss.next_blocks[0], 
        D.ss.next_blocks[1],
        D.ss.next_blocks[2],
        D.ss.next_blocks[3], round);

    // put the result into the first four values of packed_blocks
    D.ss.packed_blocks[round] = (
        (D.ss.next_blocks[0]<<6) |
        (D.ss.next_blocks[1]<<4) | 
        (D.ss.next_blocks[2]<<2) |
        (D.ss.next_blocks[3]<<0)
    );
}

void decode_blocks(int round)
{
    uint8_t block = D.ss.packed_blocks[round];
    D.ss.next_blocks[0] = (block>>6) & 3;
    D.ss.next_blocks[1] = (block>>4) & 3;
    D.ss.next_blocks[2] = (block>>2) & 3;
    D.ss.next_blocks[3] = (block>>0) & 3;
}


void simone_enter_level(int l)
{
    message(" level = %d\n", l);
    cursor.sprite->x = -16;

    level = l;
    delayed_level = 0;
    if (level>=REAL_LEVEL) 
    {
        //actual_array = (int*)malloc(xblocks*yblocks * sizeof(int));
        tmap_blit(bg,0,0, tmap_header, tmap_tmap[3*game+2]);
        // music player stop
        ply_init(0,0);

        D.ss.memorization = 1;
        D.ss.final_round = 0;  // how many rounds have we gotten up to
        D.ss.final_note = 1;   // how many notes we've gotten up to.
        delayed_level = 2;
        if (!pause)
            pause = 2;
    }
    else
    {
        // copy background into vram
        tmap_blit(bg,0,0, tmap_header, tmap_tmap[3*game+level]);
        if (level == 0)
        {
            // initialize things
            score = 0;
            pack_next_blocks(0); // get the first set of blocks into "D.ss.packed_blocks"
            // add in high score
            if (high_score[game] > 0)
            {
                // paste high score on level 0, too
                int j = 16;
                int i = SCREEN_X/2-5;
                vram[j][i--] = tmap_bg;
                vram[j++][i] = tmap_bg;
                vram[j][i++] = tmap_bg;
                vram[j][i++] = tmap_bg;
                // add in high score text
                vram[j-1][i] = tmap_bg;
                vram[j][i++] = tmap_h;
                vram[j-1][i] = tmap_bg;
                vram[j][i++] = tmap_i;
                vram[j-1][i] = tmap_bg;
                vram[j][i++] = tmap_g;
                vram[j-1][i] = tmap_bg;
                vram[j][i++] = tmap_h;
                vram[j-1][i] = tmap_bg;
                vram[j][i++] = tmap_bg;
                vram[j-1][i] = tmap_bg;
                vram[j][i++] = tmap_s;
                vram[j-1][i] = tmap_bg;
                vram[j][i++] = tmap_c;
                vram[j-1][i] = tmap_bg;
                vram[j][i++] = tmap_o;
                vram[j-1][i] = tmap_bg;
                vram[j][i++] = tmap_r;
                vram[j-1][i] = tmap_bg;
                vram[j][i++] = tmap_e;
                vram[j-1][i] = tmap_bg;
                vram[j][i++] = tmap_colon;
                vram[j-1][i] = tmap_bg;
                vram[j][i] = tmap_bg;
                
                LUINT score_breakdown = high_score[game];
                i = SCREEN_X/2+10;
                j = 19;
                while (i >= SCREEN_X/2-8)
                {
                    vram[j+2][i] = tmap_bg;
                    vram[j+1][i] = tmap_bg;
                    vram[j-1][i] = tmap_bg;
                    vram[j][i--] = tmap_zero + (score_breakdown % 10);
                    score_breakdown /= 10;
                }
            }
        }
    }
}

void simone_game_init( void ) 
{
    simone_enter_level(START_LEVEL);
    //ply_init(SONGLEN,songdata);
}

void play_index(uint8_t color_index)
{
    switch (color_index)
    {
    case 0:
        PLAY(note_right);
        break;
    case 1:
        PLAY(note_left);
        break;
    case 2:
        PLAY(note_up);
        break;
    case 3:
        PLAY(note_down);
        break;
    }
}

int simone_game_frame(void) 
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
                update_round();

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
                for (int i=0; i<4; i++)
                    set_block(SCREEN_Y/2-1, SCREEN_X/2-4 + 2*i, 5);
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
                    update_round();
                    for (int i=1; i<4; i++)
                        set_block(SCREEN_Y/2-1, SCREEN_X/2-4 + 2*i, 5);
                    decode_blocks(D.ss.current_round);
                    set_block(SCREEN_Y/2-1, SCREEN_X/2-4 + 2*D.ss.current_note, D.ss.next_blocks[D.ss.current_note]);
                    play_index(D.ss.next_blocks[D.ss.current_note]);
                    ++D.ss.current_note;
                }
                else
                {
                    set_block(SCREEN_Y/2-1, SCREEN_X/2-4 + 2*D.ss.current_note, D.ss.next_blocks[D.ss.current_note]);
                    play_index(D.ss.next_blocks[D.ss.current_note]);
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
                set_block(SCREEN_Y/2-1, SCREEN_X/2-4 + 2*D.ss.current_note, D.ss.next_blocks[D.ss.current_note]);
                if (PRESSED(0,up))
                {
                    if (D.ss.next_blocks[D.ss.current_note] == 2)
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
                    if (D.ss.next_blocks[D.ss.current_note] == 3)
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
                    if (D.ss.next_blocks[D.ss.current_note] == 0)
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
                    if (D.ss.next_blocks[D.ss.current_note] == 1)
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
                
                // if we are in a round after the first one, remove all
                // other blocks on screen besides the first one we just played.
                if (D.ss.current_note == 1)
                {
                    update_round();
                    for (int i=1; i<4; i++)
                        set_block(SCREEN_Y/2-1, SCREEN_X/2-4 + 2*i, 5);
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


