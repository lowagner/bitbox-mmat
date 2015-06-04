#include <bitbox.h>
#include <blitter.h>
#include <math.h> // sqrtf

#include <chiptune_player.h>
#include "song.h"

#include "build/tmap.h"

#include <stdlib.h> // srand rand

#define UINT unsigned int
#define LUINT unsigned long long int
#define SCREEN_X 40
#define SCREEN_Y 30
#define UNPRESS(id, key) gamepad_buttons[id] -= gamepad_##key

uint8_t vram[SCREEN_Y][SCREEN_X];
extern char build_sprite_spr[];
extern const unsigned char songdata[];

object *bg;

typedef struct _sij
{
    int8_t di, dj;
    object* sprite;
} sij;

// game state
// ----

int level, delayed_level; // level 0 : intro, level 1 : game menu, next levels : games
int time_remaining, pause, start_time;
sij cursor;// player cursor position in the matrix.
uint8_t memorization; // time to memorize or play
uint8_t nblocks_x, nblocks_y, next_nblocks_x, next_nblocks_y;
int memorization_time; // # of seconds for memorization
LUINT score, best_score, next_best_score, high_score; // best score for the layout
uint8_t blocks_not_prepped; // ready for next layout or not
uint8_t next_ncol;
uint8_t colorbucket[4];
uint8_t max_incorrect; // whether there can be more than one distinguishable switch possible

// constants
// ----


#define STARTING_SCORE 0 // change your starting score if you want to make it more difficult right away
#define START_LEVEL 0
#define REAL_LEVEL 2
#define GRAVITY .08f
#define DAMPING .8f;
#define DAMP_MUCH .5f;
#define DAMP_LITTLE .9f;
#define ELECT 700.f
#define SLOW 0.5f
#define NORMAL 2.0f
#define FAST 4.0f
#define OUT_OF_CONTROL 32.5f 
#define MAX_NBLOCKS_X 8
#define MAX_NBLOCKS_Y 4
#define BLOCKS_MAKE_NEXT 1
#define BLOCKS_CHECK_NEXT 3
#define BLOCKS_CHECK_NEXT2 5
#define BLOCKS_MAKE_JUMBLE 2
#define BLOCKS_CHECK_JUMBLE 4

const unsigned int gamepad_dpad = 15 << 8;

// constant arrays
// ----

uint8_t blocks[MAX_NBLOCKS_Y][MAX_NBLOCKS_X];
uint8_t next_blocks[MAX_NBLOCKS_Y][MAX_NBLOCKS_X];

// code
// ----

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
    int index = 2*(color)+1;
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

    // move two into where 1 was
    int index2 = vram[j2][i2]; 
    vram[j1][i1] = index2++;
    vram[j1][i1+1] = index2;
    index2 += 7;
    vram[j1+1][i1] = index2++;
    vram[j1+1][i1+1] = index2;
   
    vram[j2][i2] = index1++;
    vram[j2][i2+1] = index1;
    index1 += 7;
    vram[j2+1][i2] = index1++;
    vram[j2+1][i2+1] = index1;
}


void start_memorize()
{
    // this is where exclamation point was on timer
    vram[SCREEN_Y-5][SCREEN_X-4] = tmap_bg; // clear it out

    // also write the score
    int j = 3;
    int i = SCREEN_X/2+9;
    LUINT score_breakdown = score;
    while (score_breakdown)
    {
        vram[j][i--] = tmap_zero + (score_breakdown % 10);
        score_breakdown /= 10;
    }

    cursor.sprite->x = -16;

    // and write the fact of what you should do...
    j = 7;
    i = SCREEN_X/2-4;
    vram[j][i++] = tmap_m;
    vram[j][i++] = tmap_e;
    vram[j][i++] = tmap_m;
    vram[j][i++] = tmap_o;
    vram[j][i++] = tmap_r;
    vram[j][i++] = tmap_i;
    vram[j][i++] = tmap_z;
    vram[j][i++] = tmap_e;
    vram[j][i++] = tmap_exclamation;
   

    // get blocks setup
    best_score = next_best_score;
    nblocks_y = next_nblocks_y;
    nblocks_x = next_nblocks_x;
    for (uint8_t j=0; j<nblocks_y; j++)
    for (uint8_t i=0; i<nblocks_x; i++)
        blocks[j][i] = next_blocks[j][i];
    // write down blocks
    // upper left j,i coordinates:
    int iUL = SCREEN_X/2 - nblocks_x; // - (nblocks_x % 2);
    int jUL = SCREEN_Y/2 - nblocks_y; // - (nblocks_y % 2);
    for (uint8_t dj=0; dj<nblocks_y; dj++)
    for (uint8_t di=0; di<nblocks_x; di++)
        set_block(jUL+dj*2, iUL+di*2, blocks[dj][di]);
    
    time_remaining = memorization_time; // in real seconds
    start_time = vga_frame;
    blocks_not_prepped = BLOCKS_MAKE_JUMBLE;
    pause = 30;

    memorization = 1;
}

void start_unjumble()
{
    // this is where exclamation point was on timer
    vram[SCREEN_Y-5][SCREEN_X-4] = tmap_bg; // clear it out

    move_sprite(&cursor, nblocks_y-1, 0);
    cursor.sprite->fr=1;  // switch cursor off

    const int j = 7;
    int i = SCREEN_X/2-4;
    vram[j][i++] = tmap_u;
    vram[j][i++] = tmap_n;
    vram[j][i++] = tmap_j;
    vram[j][i++] = tmap_u;
    vram[j][i++] = tmap_m;
    vram[j][i++] = tmap_b;
    vram[j][i++] = tmap_l;
    vram[j][i++] = tmap_e;
    //vram[j][i++] = tmap_bg;
    
    // write down blocks
    // upper left j,i coordinates:
    int iUL = SCREEN_X/2 - nblocks_x; // - (nblocks_x % 2);
    int jUL = SCREEN_Y/2 - nblocks_y; // - (nblocks_y % 2);
    for (uint8_t dj=0; dj<nblocks_y; dj++)
    for (uint8_t di=0; di<nblocks_x; di++)
        set_block(jUL+dj*2, iUL+di*2, next_blocks[dj][di]);
                
    time_remaining = 3*memorization_time-1;
    start_time = vga_frame;
    pause = 30;
    memorization = 0;

    blocks_not_prepped = BLOCKS_MAKE_NEXT;
}

void score_layout()
{
    //set block code
    // make block with upper-left location indices j,i have color

    int iUL = SCREEN_X/2 - nblocks_x; // - (nblocks_x % 2);
    int jUL = SCREEN_Y/2 - nblocks_y; // - (nblocks_y % 2);
    uint8_t correct = 0;
    for (uint8_t dj=0; dj<nblocks_y; dj++)
    for (uint8_t di=0; di<nblocks_x; di++)
    {
        uint8_t color = (vram[jUL+dj*2][iUL+di*2]-1)/2;
        if (color == blocks[dj][di])
            correct += 1;
    }
    uint8_t N = (nblocks_x*nblocks_y);
    message("player got %d correct out of %d\n", correct, N);
    score += (LUINT)(1.0*correct/N * next_best_score);
    int j = 7;
    int i = SCREEN_X/2-4;
    if (correct > 3*N/4)
    {
        if (correct == N)
        {
            vram[j][i++] = tmap_p;
            vram[j][i++] = tmap_e;
            vram[j][i++] = tmap_r;
            vram[j][i++] = tmap_f;
            vram[j][i++] = tmap_e;
            vram[j][i++] = tmap_c;
            vram[j][i++] = tmap_t;
            vram[j][i++] = tmap_exclamation;
            vram[j][i++] = tmap_exclamation;
            pause = 60;
        }
        else
        {
            vram[j][i++] = tmap_bg;
            vram[j][i++] = tmap_bg;
            vram[j][i++] = tmap_g;
            vram[j][i++] = tmap_o;
            vram[j][i++] = tmap_o;
            vram[j][i++] = tmap_d;
            vram[j][i++] = tmap_exclamation;
            vram[j][i++] = tmap_bg;
            vram[j][i++] = tmap_bg;
            pause = 80;
        }
        delayed_level = level;
    }
    else
    {
        vram[j][i++] = tmap_n;
        vram[j][i++] = tmap_o;
        vram[j][i++] = tmap_t;
        vram[j][i++] = tmap_bg;
        vram[j][i++] = tmap_g;
        vram[j][i++] = tmap_o;
        vram[j][i++] = tmap_o;
        vram[j][i++] = tmap_d;
        vram[j][i++] = tmap_period;
        
        delayed_level = -1;
        pause = 150;
        if (score > high_score)
        {
            high_score = score;
        }
    }
    for (uint8_t dj=0; dj<nblocks_y; dj++)
    for (uint8_t di=0; di<nblocks_x; di++)
        set_lower_block(jUL+dj*2, iUL+di*2, blocks[dj][di]);
}

void make_board_easier()
{
    if (next_nblocks_x > 2 || next_nblocks_y > 2)
    {
        if (next_ncol == 2)
        {
            // can only decrease the board size
            if (next_nblocks_x == 1)
                // it is safe to decrement y, since one of them is greater than 2
                next_nblocks_y -= 1;
            else if (next_nblocks_y == 1)
                next_nblocks_x -= 1;
            else if (rand()%10 < 6)
                next_nblocks_x -= 1;
            else
                next_nblocks_y -= 1;
        }
        else
        {
            if (next_nblocks_x == 1)
            {
                if (rand()%10 < 8)
                    next_nblocks_y -= 1;
                else
                    next_ncol -= 1;
            }
            else if (next_nblocks_y == 1)
            {
                if (rand()%10 < 8)
                    next_nblocks_x -= 1;
                else
                    next_ncol -= 1;
            }
            else if (rand()%10 < 5)
                next_nblocks_x -= 1;
            else if (rand()%10 < 8)
                next_nblocks_y -= 1;
            else
                next_ncol -= 1;
        }
    }
    else
    {
        if (next_ncol == 2)
        {
            // just give up, we have a very small board
            blocks_not_prepped = 0;
            message("whatever, get over it\n");
            return;
        }
        else
        {
            next_ncol -= 1;
        }
    }
    blocks_not_prepped = BLOCKS_MAKE_NEXT;
}

void make_board_harder()
{
    if (next_nblocks_x*next_nblocks_y < MAX_NBLOCKS_X*MAX_NBLOCKS_Y)
    {
        if (next_ncol == 4)
        {
            // can only increase the board size
            if (next_nblocks_x == MAX_NBLOCKS_X)
                // it is safe to increment y, since N < maxx*maxy
                next_nblocks_y += 1;
            else if (next_nblocks_y == MAX_NBLOCKS_Y)
                next_nblocks_x += 1;
            else if (rand()%10 < 5)
                next_nblocks_x += 1;
            else
                next_nblocks_y += 1;
        }
        else
        {
            if (next_nblocks_x == MAX_NBLOCKS_X)
            {
                // it is safe to increment y, since N < maxx*maxy
                if (rand()%10 < 7)
                    next_nblocks_y += 1;
                else
                    next_ncol += 1;
            }
            else if (next_nblocks_y == MAX_NBLOCKS_Y)
            {
                if (rand()%10 < 7)
                    next_nblocks_x += 1;
                else
                    next_ncol += 1;
            }
            else if (rand()%10 < 5)
                next_nblocks_x += 1;
            else if (rand()%10 < 7)
                next_nblocks_y += 1;
            else
                next_ncol += 1;
        }
    }
    else
    {
        // board is huge, can only up number of colors
        if (next_ncol == 4)
        {
            // player has a super memory, he's beaten the generator
            //blocks_not_prepped = 0;
            // attempt a better one!
        }
        else
        {
            next_ncol += 1;
        }
    }
    blocks_not_prepped = BLOCKS_MAKE_NEXT;
}

void prep_next_blocks()
{
    switch (blocks_not_prepped)
    {
      case (BLOCKS_MAKE_NEXT):
      {
        message("prepping next blocks\n");
        // make our next memorization
        srand( vga_frame ); // seed the random number generator
        uint8_t colstarter = rand()%4;
        for (uint8_t i=0; i<4; i++) colorbucket[i] = 0;
        for (uint8_t j=0; j<next_nblocks_y; j++)
        for (uint8_t i=0; i<next_nblocks_x; i++)
        {
            next_blocks[j][i] = (colstarter + rand()%next_ncol)%4;
            colorbucket[next_blocks[j][i]] += 1;
        }
        blocks_not_prepped = BLOCKS_CHECK_NEXT;
        uint8_t N = next_nblocks_x*next_nblocks_y;
        uint8_t max_bucket = 0;
        for (uint8_t i=0; i<4; i++)
        {
            if (colorbucket[i] > max_bucket)
                max_bucket = colorbucket[i];
        }
        max_incorrect = 2*(N - max_bucket);
        message("max incorrect you can get in this one is %d\n",max_incorrect);
        break;
      }
      case (BLOCKS_CHECK_NEXT):
      {
        message("checking next blocks\n");
        // check whether "next_blocks" is difficult enough (and not too difficult)
        // first check color distribution:
        next_best_score = 1;
        uint8_t N = next_nblocks_x * next_nblocks_y;
        for (uint8_t i=0; i<4; i++)
            if (colorbucket[i] > 0)
                next_best_score *= (colorbucket[i])*(N - colorbucket[i]);

        if (next_best_score == 0)
        {
            // very horrible design.  somehow had all one color
            message("bad blocks\n");
            blocks_not_prepped = BLOCKS_MAKE_NEXT;
            // make it likely that won't happen again:
            if (N == 1)
                next_nblocks_x = 2;
            if (next_ncol < 4)
                next_ncol += 1;
            break;
        }

        if (next_best_score < score/4)
        {
            // board not difficult enough
            make_board_harder();
            break;
        }

        blocks_not_prepped = BLOCKS_CHECK_NEXT2;
      }
      case (BLOCKS_CHECK_NEXT2):
      {
        // things look good so far.  check for corner cases
        message("next base score is %lu\n",next_best_score);
        if (next_nblocks_y == 1)
        {
            for (uint8_t i=1; i<next_nblocks_x; i++)
                if (next_blocks[0][i-1] == next_blocks[0][i])
                    next_best_score -= 2;
        }
        else if (next_nblocks_x == 1)
        {
            for (uint8_t j=1; j<next_nblocks_y; j++)
                if (next_blocks[j-1][0] == next_blocks[j][0])
                    next_best_score -= 2;
        }
        else if (next_nblocks_y == 2)
        {
            if (next_blocks[1][0] == next_blocks[0][0])
                next_best_score -= 2;
            for (uint8_t i=1; i<next_nblocks_x; i++)
            {
                if (next_blocks[0][i-1] == next_blocks[0][i])
                    next_best_score -= 2;
                if (next_blocks[1][i-1] == next_blocks[1][i])
                    next_best_score -= 2;
                if (next_blocks[0][i-1] == next_blocks[1][i])
                    next_best_score -= 1;
                if (next_blocks[1][i-1] == next_blocks[0][i])
                    next_best_score -= 1;
                if (next_blocks[1][i] == next_blocks[0][i])
                    next_best_score -= 2;
            }
        }
        else if (next_nblocks_x == 2)
        {
            if (next_blocks[0][1] == next_blocks[0][0])
                next_best_score -= 2;
            for (uint8_t j=1; j<next_nblocks_y; j++)
            {
                if (next_blocks[j-1][0] == next_blocks[j][0])
                    next_best_score -= 2;
                if (next_blocks[j-1][1] == next_blocks[j][1])
                    next_best_score -= 2;
                if (next_blocks[j-1][0] == next_blocks[j][1])
                    next_best_score -= 1;
                if (next_blocks[j-1][1] == next_blocks[j][0])
                    next_best_score -= 1;
                if (next_blocks[j][1] == next_blocks[j][0])
                    next_best_score -= 2;
            }
        }
        else
        {
            // check the bottom row
            for (uint8_t i=1; i<next_nblocks_x; i++)
                if (next_blocks[0][i-1] == next_blocks[0][i])
                    next_best_score -= 2;

            for (uint8_t j=1; j<next_nblocks_y; j++)
            {
                // check the left column
                if (next_blocks[j-1][0] == next_blocks[j][0])
                    next_best_score -= 2;
                for (uint8_t i=1; i<next_nblocks_x; i++)
                {
                    // check the matrix
                    if (next_blocks[j][i] == next_blocks[j-1][i])
                        next_best_score -= 2;
                    if (next_blocks[j][i] == next_blocks[j][i-1])
                        next_best_score -= 2;
                    if (next_blocks[j][i-1] == next_blocks[j-1][i])
                        next_best_score -= 1;
                }
            }
        }
        message("final refined score is %lu\n",next_best_score);
        if (next_best_score < score/6)
        {
            message("oops, too easy\n");
            make_board_harder();
            break;
        }
        else if (next_best_score > (score+16))
        {
            message("oops, too hard\n");
            make_board_easier();
            break;
        }
        blocks_not_prepped = 0;
        // if not, set blocks_not_prepped to BLOCKS_MAKE_NEXT
        break;
      }
      case (BLOCKS_MAKE_JUMBLE):
      {
        message("making the jumble\n");
        // create a jumble of the current blocks
        uint8_t index = 0;
        uint8_t N = next_nblocks_x * next_nblocks_y;
        for (uint8_t j=0; j<next_nblocks_y; j++)
        for (uint8_t i=0; i<next_nblocks_x; i++)
        {
            uint8_t r = index+rand()%(N-index);
            uint8_t jr = r % next_nblocks_y;
            uint8_t ir = r / next_nblocks_y;
            // swap i,j color with ir,jr color:
            uint8_t cr = next_blocks[jr][ir];
            next_blocks[jr][ir] = next_blocks[j][i];
            next_blocks[j][i] = cr;
            index += 1;
        }
        blocks_not_prepped = BLOCKS_CHECK_JUMBLE;
        break;
      }
      case (BLOCKS_CHECK_JUMBLE):
      {
        message("checking the jumble\n");
        // check whether jumble is different enough
        int8_t incorrect = 0;
        for (uint8_t j=0; j<nblocks_y; j++)
        for (uint8_t i=0; i<nblocks_x; i++)
        {
            if (next_blocks[j][i] != blocks[j][i])
                incorrect += 1;
        }
        int8_t N = next_nblocks_x * next_nblocks_y;
        if ((incorrect <= max_incorrect/2)
           && (incorrect <= N/2) )
            blocks_not_prepped = BLOCKS_MAKE_JUMBLE;
        else
            // it'll be tolerable
            blocks_not_prepped = 0;
        break;
      }
    }
}


void enter_level(int l)
{
    message(" level = %d\n", l);

    level = l;
    if (level>=REAL_LEVEL) 
    {
        //actual_array = (int*)malloc(xblocks*yblocks * sizeof(int));
        tmap_blit(bg,0,0, tmap_header, tmap_tmap[2]);
        // music player stop
        ply_init(0,0);
        start_memorize(); // also lays out the blocks defined here.
    }
    else
    {
        // copy background into vram
        tmap_blit(bg,0,0, tmap_header, tmap_tmap[level]);
        if (level == 0)
        {
            // reset everything
            blocks_not_prepped = BLOCKS_MAKE_NEXT;
            next_nblocks_x = 2;
            next_nblocks_y = 2;
            next_ncol = 2;
            nblocks_x = 2;
            nblocks_y = 2;
            memorization_time = 20;
            delayed_level = 0;
            // reset score
            score = STARTING_SCORE;
            cursor.sprite->x = -16;
            if (high_score > 0)
            {
                // paste high score on level 0, too
                int j = 16;
                int i = SCREEN_X/2-5;
                // clear out blocks on left
                vram[j][i--] = tmap_bg;
                vram[j++][i] = tmap_bg;
                vram[j][i++] = tmap_bg;
                vram[j][i++] = tmap_bg;
                // add in high score text
                vram[j][i++] = tmap_h;
                vram[j][i++] = tmap_i;
                vram[j][i++] = tmap_g;
                vram[j][i++] = tmap_h;
                vram[j][i++] = tmap_bg;
                vram[j][i++] = tmap_s;
                vram[j][i++] = tmap_c;
                vram[j][i++] = tmap_o;
                vram[j][i++] = tmap_r;
                vram[j][i++] = tmap_e;
                vram[j][i++] = tmap_colon;
                // clear out blocks on the right
                vram[j][i++] = tmap_bg;
                vram[j--][i] = tmap_bg;
                vram[j][i--] = tmap_bg;
                vram[j][i--] = tmap_bg;
                vram[j][i--] = tmap_bg;
                vram[j][i--] = tmap_bg;
                vram[j][i] = tmap_bg;
                
                LUINT score_breakdown = high_score;
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
        else // level 1 (tutorial)
        {
            // in case player starts with score>0
            // we need to override blocks to 2x2, so that move_sprite works well.
            uint8_t bx, by;
            bx = nblocks_x;
            by = nblocks_y;
            nblocks_x = 2;
            nblocks_y = 2;
            move_sprite(&cursor, nblocks_y,-5);
            // set them back at the end, however, so that play can continue.
            nblocks_x = bx;
            nblocks_y = by;
            cursor.sprite->fr=1;
        }
        //selection.sprite->x = -16;
        pause = 30;
    }

    delayed_level = 0;

    // remove start button presses here (doube-assurance I suppose):
    if GAMEPAD_PRESSED(0, start)
        UNPRESS(0, start);
}

void game_init( void ) 
{
    high_score = 0;

    blitter_init(); // screen_x was 64 below
    bg = tilemap_new(tmap_tset,0,0,TMAP_HEADER(SCREEN_X,SCREEN_Y,TSET_16, TMAP_U8), vram); 
    cursor.sprite = sprite_new(build_sprite_spr,0,0,0);
    //selection.sprite = sprite_new(build_sprite_spr,0,0,0);
    
    enter_level(START_LEVEL);
    ply_init(SONGLEN,songdata);
}

void game_frame( void ) {

    ply_update();

    if (pause)
    {
        pause--;
        if (GAMEPAD_PRESSED(0,start) && pause > 5) 
        {
            pause = 5;
            UNPRESS(0, start);
        }
        if (!pause)
            if (delayed_level)
                enter_level(delayed_level+1);
        return;
    }

    kbd_emulate_gamepad();

    if (blocks_not_prepped)
        prep_next_blocks();

    if (level < REAL_LEVEL) 
    {  // men
        if (level == 0 && vga_frame % 24==0)
            // animate tmaps : cycle squares
            for (int j=0;j<SCREEN_Y;j++) 
                for (int i=0;i<SCREEN_X;i++) 
                    switch (vram[j][i])
                    {
                    case tmap_yellow_UL:  vram[j][i] = tmap_yellow_DL; break;
                    case tmap_yellow_UR:  vram[j][i] = tmap_yellow_DR; break;
                    case tmap_yellow_DL:  vram[j][i] = tmap_green_UL; break;
                    case tmap_yellow_DR:  vram[j][i] = tmap_green_UR; break;
                    case tmap_green_UL:  vram[j][i] = tmap_green_DL; break;
                    case tmap_green_UR:  vram[j][i] = tmap_green_DR; break;
                    case tmap_green_DL:  vram[j][i] = tmap_yellow_UL; break;
                    case tmap_green_DR:  vram[j][i] = tmap_yellow_UR; break;
                    }
        else if (level == 1 && vga_frame % 48 == 0)
        {
            // move cursor around and simulate intelligence
            switch (vga_frame % 5)
            {
            case 0:
                cursor.dj -= 1;
                cursor.sprite->y -= 32;
                cursor.sprite->fr=1; 
                break;
            case 1:
                cursor.di += 1;
                cursor.sprite->x += 32;
                cursor.sprite->fr=0; 
                break;
            case 2:
                cursor.di -= 1;
                swap_blocks(cursor.dj, cursor.di, cursor.dj, cursor.di+1);
                cursor.sprite->x -= 32;
                break;
            case 3:
                cursor.dj += 1;
                cursor.sprite->y += 32;
                break;
            default:
                break;
            }
        }

        // just wait for keypress - wait a bit ?
        if (GAMEPAD_PRESSED(0,start))
        {
            UNPRESS(0, start);
            enter_level(level+1);
        }
        else if (GAMEPAD_PRESSED(0,A)) 
        {
            enter_level(level+1);
        }
    } 
    else 
    {
        // show time since beginning
        int t=time_remaining - (vga_frame-start_time)/60;
        if (t <= 0 || GAMEPAD_PRESSED(0, start))
        {
            if (GAMEPAD_PRESSED(0, start))
                UNPRESS(0, start);
            if (memorization)
            {
                start_unjumble();
                return;
            }
            else
            {
                score_layout();
//                    enter_level(level+1);
//                else
//                    enter_level(0); // maybe enter -1 level for lose level
//                return;
                return;
            }
        }
        vram[SCREEN_Y-5][SCREEN_X-7] = tmap_zero + (t/100)%10;
        vram[SCREEN_Y-5][SCREEN_X-6] = tmap_zero + (t/10)%10;
        vram[SCREEN_Y-5][SCREEN_X-5] = tmap_zero + t%10;
        if (t < 10)
        {
            int frame = (vga_frame % 50);
            if (frame == 25)
                vram[SCREEN_Y-5][SCREEN_X-4] = tmap_exclamation;
            else if (frame == 0)
                vram[SCREEN_Y-5][SCREEN_X-4] = tmap_bg;
        }

        if (!memorization)
        {
            // input handling
            if (GAMEPAD_PRESSED(0,B)) 
                cursor.sprite->fr=0; 
            else
                cursor.sprite->fr=1; 
            if (gamepad_buttons[0] & gamepad_dpad)
            {
                if (GAMEPAD_PRESSED(0,up))
                {
                    if (cursor.dj > 0)
                    {
                        cursor.dj -= 1;
                        cursor.sprite->y -= 32;
                        if (GAMEPAD_PRESSED(0, B))
                            swap_blocks(cursor.dj, cursor.di, cursor.dj+1, cursor.di);
                            // don't unpress here, because we want player to be able
                            // to chain moving things around
                    }
                    UNPRESS(0, up);
                }
                else if (GAMEPAD_PRESSED(0, down))
                {
                    if (cursor.dj < nblocks_y-1)
                    {
                        cursor.dj += 1;
                        cursor.sprite->y += 32;
                        if (GAMEPAD_PRESSED(0, B))
                            swap_blocks(cursor.dj, cursor.di, cursor.dj-1, cursor.di);
                    }
                    UNPRESS(0, down);
                }
                else if (GAMEPAD_PRESSED(0, right))
                {
                    if (cursor.di < nblocks_x-1)
                    {
                        cursor.di += 1;
                        cursor.sprite->x += 32;
                        if (GAMEPAD_PRESSED(0, B))
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
                        if (GAMEPAD_PRESSED(0, B))
                            swap_blocks(cursor.dj, cursor.di, cursor.dj, cursor.di+1);
                    }
                    UNPRESS(0, left);
                }
            }
        }
    }
}
