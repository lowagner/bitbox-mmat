#include "memmat.h"
#include "common.h"

#define STARTING_SCORE 0 // change your starting score if you want to make it more difficult right away
#define MAX_NBLOCKS_X 8
#define MAX_NBLOCKS_Y 4
#define BLOCKS_MAKE_NEXT 1
#define BLOCKS_CHECK_NEXT 3
#define BLOCKS_CHECK_NEXT2 5
#define BLOCKS_MAKE_JUMBLE 2
#define BLOCKS_CHECK_JUMBLE 4


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
    D.mm.best_score = D.mm.next_best_score;
    D.mm.nblocks_y = D.mm.next_nblocks_y;
    D.mm.nblocks_x = D.mm.next_nblocks_x;
    for (int j=0; j<D.mm.nblocks_y; j++)
    for (int i=0; i<D.mm.nblocks_x; i++)
        D.mm.blocks[j][i] = D.mm.next_blocks[j][i];
    // write down blocks
    // upper left j,i coordinates:
    int iUL = SCREEN_X/2 - D.mm.nblocks_x; // - (nblocks_x % 2);
    int jUL = SCREEN_Y/2 - D.mm.nblocks_y; // - (nblocks_y % 2);
    for (int dj=0; dj<D.mm.nblocks_y; dj++)
    for (int di=0; di<D.mm.nblocks_x; di++)
        set_block(jUL+dj*2, iUL+di*2, D.mm.blocks[dj][di]);
    
    D.mm.time_remaining = D.mm.memorization_time; // in real seconds
    D.mm.start_time = vga_frame;
    D.mm.blocks_not_prepped = BLOCKS_MAKE_JUMBLE;
    pause = 10;

    D.mm.memorization = 1;
}

void start_unjumble()
{
    // this is where exclamation point was on timer
    vram[SCREEN_Y-5][SCREEN_X-4] = tmap_bg; // clear it out

    move_sprite_extra(&cursor, D.mm.nblocks_y-1, 0, D.mm.nblocks_x, D.mm.nblocks_y);
    cursor.sprite->fr=0;  // switch cursor off

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
    int iUL = SCREEN_X/2 - D.mm.nblocks_x; // - (nblocks_x % 2);
    int jUL = SCREEN_Y/2 - D.mm.nblocks_y; // - (nblocks_y % 2);
    for (int dj=0; dj<D.mm.nblocks_y; dj++)
    for (int di=0; di<D.mm.nblocks_x; di++)
        set_block(jUL+dj*2, iUL+di*2, D.mm.next_blocks[dj][di]);
                
    D.mm.time_remaining = 3*D.mm.memorization_time-1;
    D.mm.start_time = vga_frame;
    pause = 10;
    D.mm.memorization = 0;

    D.mm.blocks_not_prepped = BLOCKS_MAKE_NEXT;
}

void score_layout()
{
    //set block code
    // make block with upper-left location indices j,i have color

    int iUL = SCREEN_X/2 - D.mm.nblocks_x; // - (nblocks_x % 2);
    int jUL = SCREEN_Y/2 - D.mm.nblocks_y; // - (nblocks_y % 2);
    int correct = 0;
    for (int dj=0; dj<D.mm.nblocks_y; dj++)
    for (int di=0; di<D.mm.nblocks_x; di++)
    {
        uint8_t color = vram[jUL+dj*2][iUL+di*2]/2;
        if (color == D.mm.blocks[dj][di])
            correct += 1;
    }
    int N = (D.mm.nblocks_x*D.mm.nblocks_y);
    message("player got %d correct out of %d\n", correct, N);
    score += (LUINT)(1.0*correct/N * D.mm.best_score);
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
        pause = 180;
        if (score > high_score[game])
        {
            high_score[game] = score;
        }
    }
    for (int dj=0; dj<D.mm.nblocks_y; dj++)
    for (int di=0; di<D.mm.nblocks_x; di++)
        set_lower_block(jUL+dj*2, iUL+di*2, D.mm.blocks[dj][di]);
}

void make_board_easier()
{
    if (D.mm.next_nblocks_x > 2 || D.mm.next_nblocks_y > 2)
    {
        if (D.mm.next_ncol == 2)
        {
            // can only decrease the board size
            if (D.mm.next_nblocks_x == 1)
                // it is safe to decrement y, since one of them is greater than 2
                D.mm.next_nblocks_y -= 1;
            else if (D.mm.next_nblocks_y == 1)
                D.mm.next_nblocks_x -= 1;
            else if (rand()%10 < 6)
                D.mm.next_nblocks_x -= 1;
            else
                D.mm.next_nblocks_y -= 1;
        }
        else
        {
            if (D.mm.next_nblocks_x == 1)
            {
                if (rand()%10 < 8)
                    D.mm.next_nblocks_y -= 1;
                else
                    D.mm.next_ncol -= 1;
            }
            else if (D.mm.next_nblocks_y == 1)
            {
                if (rand()%10 < 8)
                    D.mm.next_nblocks_x -= 1;
                else
                    D.mm.next_ncol -= 1;
            }
            else if (rand()%10 < 5)
                D.mm.next_nblocks_x -= 1;
            else if (rand()%10 < 8)
                D.mm.next_nblocks_y -= 1;
            else
                D.mm.next_ncol -= 1;
        }
    }
    else
    {
        if (D.mm.next_ncol == 2)
        {
            // just give up, we have a very small board
            D.mm.blocks_not_prepped = 0;
            message("whatever, get over it\n");
            return;
        }
        else
        {
            D.mm.next_ncol -= 1;
        }
    }
    D.mm.blocks_not_prepped = BLOCKS_MAKE_NEXT;
}

void make_board_harder()
{
    if (D.mm.next_nblocks_x*D.mm.next_nblocks_y < MAX_NBLOCKS_X*MAX_NBLOCKS_Y)
    {
        if (D.mm.next_ncol == 4)
        {
            // can only increase the board size
            if (D.mm.next_nblocks_x == MAX_NBLOCKS_X)
                // it is safe to increment y, since N < maxx*maxy
                D.mm.next_nblocks_y += 1;
            else if (D.mm.next_nblocks_y == MAX_NBLOCKS_Y)
                D.mm.next_nblocks_x += 1;
            else if (rand()%10 < 5)
                D.mm.next_nblocks_x += 1;
            else
                D.mm.next_nblocks_y += 1;
        }
        else
        {
            if (D.mm.next_nblocks_x == MAX_NBLOCKS_X)
            {
                // it is safe to increment y, since N < maxx*maxy
                if (rand()%10 < 7)
                    D.mm.next_nblocks_y += 1;
                else
                    D.mm.next_ncol += 1;
            }
            else if (D.mm.next_nblocks_y == MAX_NBLOCKS_Y)
            {
                if (rand()%10 < 7)
                    D.mm.next_nblocks_x += 1;
                else
                    D.mm.next_ncol += 1;
            }
            else if (rand()%10 < 5)
                D.mm.next_nblocks_x += 1;
            else if (rand()%10 < 7)
                D.mm.next_nblocks_y += 1;
            else
                D.mm.next_ncol += 1;
        }
    }
    else
    {
        // board is huge, can only up number of colors
        if (D.mm.next_ncol == 4)
        {
            // player has a super memory, he's beaten the generator
            //blocks_not_prepped = 0;
            // attempt a better one!
        }
        else
        {
            D.mm.next_ncol += 1;
        }
    }
    D.mm.blocks_not_prepped = BLOCKS_MAKE_NEXT;
}

void prep_next_blocks()
{
    switch (D.mm.blocks_not_prepped)
    {
      case (BLOCKS_MAKE_NEXT):
      {
        message("prepping next blocks\n");
        // make our next memorization
        srand(vga_frame); // seed the random number generator
        uint8_t colstarter = rand()%4;
        for (int i=0; i<4; i++) D.mm.colorbucket[i] = 0;
        for (int j=0; j<D.mm.next_nblocks_y; j++)
        for (int i=0; i<D.mm.next_nblocks_x; i++)
        {
            D.mm.next_blocks[j][i] = (colstarter + rand()%D.mm.next_ncol)%4;
            D.mm.colorbucket[D.mm.next_blocks[j][i]] += 1;
        }
        D.mm.blocks_not_prepped = BLOCKS_CHECK_NEXT;
        int N = D.mm.next_nblocks_x*D.mm.next_nblocks_y;
        uint8_t max_bucket = 0;
        for (int i=0; i<4; i++)
        {
            if (D.mm.colorbucket[i] > max_bucket)
                max_bucket = D.mm.colorbucket[i];
        }
        D.mm.max_incorrect = 2*(N - max_bucket);
        message("max incorrect you can get in this one is %d\n",D.mm.max_incorrect);
        break;
      }
      case (BLOCKS_CHECK_NEXT):
      {
        message("checking next blocks\n");
        // check whether "next_blocks" is difficult enough (and not too difficult)
        // first check color distribution:
        D.mm.next_best_score = 1;
        int N = D.mm.next_nblocks_x * D.mm.next_nblocks_y;
        for (int i=0; i<4; i++)
            if (D.mm.colorbucket[i] > 0)
                D.mm.next_best_score *= (D.mm.colorbucket[i])*(N - D.mm.colorbucket[i]);

        if (D.mm.next_best_score == 0)
        {
            // very horrible design.  somehow had all one color
            message("bad blocks\n");
            D.mm.blocks_not_prepped = BLOCKS_MAKE_NEXT;
            // make it likely that won't happen again:
            if (N == 1)
                D.mm.next_nblocks_x = 2;
            if (D.mm.next_ncol < 4)
                D.mm.next_ncol += 1;
            break;
        }

        if (D.mm.next_best_score < score/4)
        {
            // board not difficult enough
            make_board_harder();
            break;
        }

        D.mm.blocks_not_prepped = BLOCKS_CHECK_NEXT2;
      }
      case (BLOCKS_CHECK_NEXT2):
      {
        // things look good so far.  check for corner cases
        message("next base score is %lu\n",D.mm.next_best_score);
        if (D.mm.next_nblocks_y == 1)
        {
            for (int i=1; i<D.mm.next_nblocks_x; i++)
                if (D.mm.next_blocks[0][i-1] == D.mm.next_blocks[0][i])
                    D.mm.next_best_score -= 2;
        }
        else if (D.mm.next_nblocks_x == 1)
        {
            for (int j=1; j<D.mm.next_nblocks_y; j++)
                if (D.mm.next_blocks[j-1][0] == D.mm.next_blocks[j][0])
                    D.mm.next_best_score -= 2;
        }
        else if (D.mm.next_nblocks_y == 2)
        {
            if (D.mm.next_blocks[1][0] == D.mm.next_blocks[0][0])
                D.mm.next_best_score -= 2;
            for (int i=1; i<D.mm.next_nblocks_x; i++)
            {
                if (D.mm.next_blocks[0][i-1] == D.mm.next_blocks[0][i])
                    D.mm.next_best_score -= 2;
                if (D.mm.next_blocks[1][i-1] == D.mm.next_blocks[1][i])
                    D.mm.next_best_score -= 2;
                if (D.mm.next_blocks[0][i-1] == D.mm.next_blocks[1][i])
                    D.mm.next_best_score -= 1;
                if (D.mm.next_blocks[1][i-1] == D.mm.next_blocks[0][i])
                    D.mm.next_best_score -= 1;
                if (D.mm.next_blocks[1][i] == D.mm.next_blocks[0][i])
                    D.mm.next_best_score -= 2;
            }
        }
        else if (D.mm.next_nblocks_x == 2)
        {
            if (D.mm.next_blocks[0][1] == D.mm.next_blocks[0][0])
                D.mm.next_best_score -= 2;
            for (int j=1; j<D.mm.next_nblocks_y; j++)
            {
                if (D.mm.next_blocks[j-1][0] == D.mm.next_blocks[j][0])
                    D.mm.next_best_score -= 2;
                if (D.mm.next_blocks[j-1][1] == D.mm.next_blocks[j][1])
                    D.mm.next_best_score -= 2;
                if (D.mm.next_blocks[j-1][0] == D.mm.next_blocks[j][1])
                    D.mm.next_best_score -= 1;
                if (D.mm.next_blocks[j-1][1] == D.mm.next_blocks[j][0])
                    D.mm.next_best_score -= 1;
                if (D.mm.next_blocks[j][1] == D.mm.next_blocks[j][0])
                    D.mm.next_best_score -= 2;
            }
        }
        else
        {
            // check the bottom row
            for (int i=1; i<D.mm.next_nblocks_x; i++)
                if (D.mm.next_blocks[0][i-1] == D.mm.next_blocks[0][i])
                    D.mm.next_best_score -= 2;

            for (int j=1; j<D.mm.next_nblocks_y; j++)
            {
                // check the left column
                if (D.mm.next_blocks[j-1][0] == D.mm.next_blocks[j][0])
                    D.mm.next_best_score -= 2;
                for (int i=1; i<D.mm.next_nblocks_x; i++)
                {
                    // check the matrix
                    if (D.mm.next_blocks[j][i] == D.mm.next_blocks[j-1][i])
                        D.mm.next_best_score -= 2;
                    if (D.mm.next_blocks[j][i] == D.mm.next_blocks[j][i-1])
                        D.mm.next_best_score -= 2;
                    if (D.mm.next_blocks[j][i-1] == D.mm.next_blocks[j-1][i])
                        D.mm.next_best_score -= 1;
                }
            }
        }
        message("final refined score is %lu\n",D.mm.next_best_score);
        if (D.mm.next_best_score < score/6)
        {
            message("oops, too easy\n");
            make_board_harder();
            break;
        }
        else if (D.mm.next_best_score > (score/2+16))
        {
            message("oops, too hard\n");
            make_board_easier();
            break;
        }
        D.mm.blocks_not_prepped = 0;
        // if not, set blocks_not_prepped to BLOCKS_MAKE_NEXT
        break;
      }
      case (BLOCKS_MAKE_JUMBLE):
      {
        message("making the jumble\n");
        // create a jumble of the current blocks
        int index = 0;
        int N = D.mm.next_nblocks_x * D.mm.next_nblocks_y;
        for (int j=0; j<D.mm.next_nblocks_y; j++)
        for (int i=0; i<D.mm.next_nblocks_x; i++)
        {
            int r = index+rand()%(N-index);
            int jr = r / D.mm.next_nblocks_x;
            int ir = r % D.mm.next_nblocks_x;
            // swap i,j color with ir,jr color:
            uint8_t cr = D.mm.next_blocks[jr][ir];
            D.mm.next_blocks[jr][ir] = D.mm.next_blocks[j][i];
            D.mm.next_blocks[j][i] = cr;
            index += 1;
        }
        D.mm.blocks_not_prepped = BLOCKS_CHECK_JUMBLE;
        break;
      }
      case (BLOCKS_CHECK_JUMBLE):
      {
        message("checking the jumble\n");
        // check whether jumble is different enough
        uint8_t incorrect = 0;
        for (int j=0; j<D.mm.nblocks_y; j++)
        for (int i=0; i<D.mm.nblocks_x; i++)
        {
            if (D.mm.next_blocks[j][i] != D.mm.blocks[j][i])
                incorrect += 1;
        }
        int N = D.mm.next_nblocks_x * D.mm.next_nblocks_y;
        if ((incorrect <= D.mm.max_incorrect/2)
           && (incorrect <= N/2) )
            D.mm.blocks_not_prepped = BLOCKS_MAKE_JUMBLE;
        else
            // it'll be tolerable
            D.mm.blocks_not_prepped = 0;
        break;
      }
    }
}


void memmat_enter_level(int l)
{
    message(" level = %d\n", l);

    level = l;
    if (level>=REAL_LEVEL) 
    {
        //actual_array = (int*)malloc(xblocks*yblocks * sizeof(int));
        tmap_blit(bg,0,0, tmap_header, tmap_tmap[3*game+2]);
        // music player stop
        STOP;
        start_memorize(); // also lays out the blocks defined here.
    }
    else
    {
        // copy background into vram
        tmap_blit(bg,0,0, tmap_header, tmap_tmap[3*game+level]);
        if (level == 0)
        {
            // reset everything
            D.mm.blocks_not_prepped = BLOCKS_MAKE_NEXT;
            D.mm.next_nblocks_x = 2;
            D.mm.next_nblocks_y = 2;
            D.mm.next_ncol = 2;
            D.mm.nblocks_x = 2;
            D.mm.nblocks_y = 2;
            D.mm.memorization_time = 20;
            // reset score
            score = STARTING_SCORE;
            cursor.sprite->x = -16;
            if (high_score[game] > 0)
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
        else // level 1 (tutorial)
        {
            move_sprite_extra(&cursor, 2,-5, 2, 2);
            cursor.sprite->fr=0;
        }
    }

    delayed_level = 0;
}

void memmat_game_init( void ) 
{
    memmat_enter_level(START_LEVEL);
    //ply_init(SONGLEN,songdata);
}

int memmat_game_frame(void) 
{

    ply_update();

    read_presses();

    if (D.mm.blocks_not_prepped)
        prep_next_blocks();

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
                memmat_enter_level(delayed_level+1);
        return 0;
    }

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
                cursor.sprite->fr=0; 
                break;
            case 1:
                cursor.di += 1;
                cursor.sprite->x += 32;
                cursor.sprite->fr=1; 
                break;
            case 2:
                cursor.di -= 1;
                swap_blocks_extra(cursor.dj, cursor.di, cursor.dj, cursor.di+1, D.mm.nblocks_x, D.mm.nblocks_y);
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
        if (PRESSED(0,start))
        {
            my_gamepad_buttons[0] = 0;
            memmat_enter_level(level+1);
        }
        else if (PRESSED(0, select))
        {
            my_gamepad_buttons[0] = 0;
            if (level == 0)
                return 1;
            else
                memmat_enter_level(0);
            return 0;
        }
        else if (PRESSED(0,any)) 
        {
            my_gamepad_buttons[0] = 0;
            memmat_enter_level(level+1);
        }
    } 
    else 
    {
        // show time since beginning
        int t=D.mm.time_remaining - (vga_frame-D.mm.start_time)/60;
        if (t == 11)
            PLAY(ticktock);
        if (t <= 0 || PRESSED(0, start))
        {
            STOP;
            if (PRESSED(0, start))
                UNPRESS(0, start);
            if (D.mm.memorization)
            {
                start_unjumble();
                return 0;
            }
            else
            {
                score_layout();
                return 0;
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

        if (!D.mm.memorization)
        {
            // input handling
            if (PRESSED(0,any)) 
                cursor.sprite->fr=1; 
            else
                cursor.sprite->fr=0; 
            if (PRESSED(0, dpad))
            {
                if (PRESSED(0,up))
                {
                    if (cursor.dj > 0)
                    {
                        cursor.dj -= 1;
                        cursor.sprite->y -= 32;
                        if (PRESSED(0, any))
                            swap_blocks_extra(cursor.dj, cursor.di, cursor.dj+1, cursor.di, D.mm.nblocks_x, D.mm.nblocks_y);
                            // don't unpress here, because we want player to be able
                            // to chain moving things around
                    }
                    UNPRESS(0, up);
                }
                else if (PRESSED(0, down))
                {
                    if (cursor.dj < D.mm.nblocks_y-1)
                    {
                        cursor.dj += 1;
                        cursor.sprite->y += 32;
                        if (PRESSED(0, any))
                            swap_blocks_extra(cursor.dj, cursor.di, cursor.dj-1, cursor.di, D.mm.nblocks_x, D.mm.nblocks_y);
                    }
                    UNPRESS(0, down);
                }
                else if (PRESSED(0, right))
                {
                    if (cursor.di < D.mm.nblocks_x-1)
                    {
                        cursor.di += 1;
                        cursor.sprite->x += 32;
                        if (PRESSED(0, any))
                            swap_blocks_extra(cursor.dj, cursor.di, cursor.dj, cursor.di-1, D.mm.nblocks_x, D.mm.nblocks_y);
                    }
                    UNPRESS(0, right);
                }
                else // left was pressed
                {
                    if (cursor.di > 0)
                    {
                        cursor.di -= 1;
                        cursor.sprite->x -= 32;
                        if (PRESSED(0, any))
                            swap_blocks_extra(cursor.dj, cursor.di, cursor.dj, cursor.di+1, D.mm.nblocks_x, D.mm.nblocks_y);
                    }
                    UNPRESS(0, left);
                }
            }
        }
    }
    return 0;
}

