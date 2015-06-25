#include "common.h"

void move_sprite(sij* sij, int8_t dj, int8_t di)
{ 
    sij->sprite->y = 8*SCREEN_Y + 16*(-nblocks_y+2*dj)+8;
    sij->sprite->x = 8*SCREEN_X + 16*(-nblocks_x+2*di)+8;
    sij->di = di;
    sij->dj = dj;
}

void set_lower_block(int j, int i, uint8_t color)
{
    // make block with upper-left location indices j,i have color
    int index = 2*(color)+9;
    vram[j+1][i] = index++;
    vram[j+1][i+1] = index;
}

void set_block(int j, int i, uint8_t color)
{
    // make block with upper-left location indices j,i have color
    if (color < 4)
    {
        int index = 2*(color)+1;
        vram[j][i] = index++;
        vram[j][i+1] = index;
        index += 7;
        vram[j+1][i] = index++;
        vram[j+1][i+1] = index;
    }
    else
    {
        vram[j][i] = tmap_bg;
        vram[j][i+1] = tmap_bg;
        vram[j+1][i] = tmap_bg;
        vram[j+1][i+1] = tmap_bg;
    }
}

void delete_block(int dj, int di)
{
    int i = di*2 + SCREEN_X/2 - nblocks_x; // - (nblocks_x % 2);
    int j = dj*2 + SCREEN_Y/2 - nblocks_y; // - (nblocks_y % 2);    
    vram[j][i] = tmap_bg;
    vram[j][i+1] = tmap_bg;
    vram[j+1][i] = tmap_bg;
    vram[j+1][i+1] = tmap_bg;
}

void create_or_cycle_block(int dj, int di, int direction)
{
    int i = di*2 + SCREEN_X/2 - nblocks_x; // - (nblocks_x % 2);
    int j = dj*2 + SCREEN_Y/2 - nblocks_y; // - (nblocks_y % 2);     
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
    }
    else
        // choose a random index
        index = (rand()%4)*2+1;
    //message("index = %d\n", index);
    vram[j][i] = index++;
    vram[j][i+1] = index;
    index += 7;
    vram[j+1][i] = index++;
    vram[j+1][i+1] = index;
}

void swap_blocks(int8_t dj1, int8_t di1, int8_t dj2, int8_t di2)
{
    int iUL = SCREEN_X/2 - nblocks_x; // - (nblocks_x % 2);
    int jUL = SCREEN_Y/2 - nblocks_y; // - (nblocks_y % 2);
    
    int j1 = jUL + dj1*2;
    int j2 = jUL + dj2*2;
    int i1 = iUL + di1*2;
    int i2 = iUL + di2*2;
    
    int index1 = vram[j1][i1]; // copy color 1 into memory
    int index2 = vram[j2][i2]; // and color2 as well!

    if (index2 < 16)
    {
        // move two into where 1 was
        vram[j1][i1] = index2++;
        vram[j1][i1+1] = index2;
        index2 += 7;
        vram[j1+1][i1] = index2++;
        vram[j1+1][i1+1] = index2;
    }
    else
    {
        vram[j1][i1] = tmap_bg;
        vram[j1][i1+1] = tmap_bg;
        vram[j1+1][i1] = tmap_bg;
        vram[j1+1][i1+1] = tmap_bg;
    }
  
    if (index1 < 16)
    {
        vram[j2][i2] = index1++;
        vram[j2][i2+1] = index1;
        index1 += 7;
        vram[j2+1][i2] = index1++;
        vram[j2+1][i2+1] = index1;
    }
    else
    { 
        vram[j2][i2] = tmap_bg;
        vram[j2][i2+1] = tmap_bg;
        vram[j2+1][i2] = tmap_bg;
        vram[j2+1][i2+1] = tmap_bg;
    }
}

void read_presses()
{
    // get current state
    kbd_emulate_gamepad();

    // update my state using only new presses:
    for (uint8_t i=0; i<2; ++i)
    {
        // only allow button on if it's on:
        my_gamepad_buttons[i] &= gamepad_buttons[i];
        // but only add in _new_ presses to my gamepad buttons
        my_gamepad_buttons[i] |= gamepad_buttons[i] & (~prev_gamepad_buttons[i]);
    }
    
    // get previous state of buttons
    for (uint8_t i=0; i<2; i++)
        prev_gamepad_buttons[i] = gamepad_buttons[i];
}

