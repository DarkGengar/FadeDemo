/**
 * Copyright (C) 2019 Joel Nauschütz (DarkGengar)
 * 
 * This file is part of Fade Demo.
 * 
 * Fade Demo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Fade Demo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Fade Demo.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * Author: Joel Nauschütz (DarkGengar)
 * Date: 01/28/2019
 * 
 */

// TODO: Implement better palette fade algorithms

#include <tonc.h>
#include <gfx.h>
#include <stdlib.h>

#define FPS 60

#define GRIT_CPY16(dst, name)	memcpy16(dst, name, name##Len/2)
#define GRIT_CPY32(dst, name)   memcpy32(dst, name, name##Len/4)
// time in seconds
#define SLEEP(time) VBlankIntrDelay(time * FPS)

void fadeout(int mode, int speed, int bg);
void fadein(int mode, int speed, int bg);

// no hardware fade, 
COLOR *fadeout_bg(int start, int end);
void fadein_bg(COLOR *fade_tbl);

INLINE int get_clr_chn(COLOR clr, int n);
INLINE void clear_pals();
INLINE void clear_tile_mem();

int main()
{
    int frame = 0;

    memcpy32(&tile_mem[0][0], bg_bootscreenTiles, bg_bootscreenTilesLen/4);
    memcpy32(&se_mem[31][0], bg_bootscreenMap, bg_bootscreenMapLen/4);
    memcpy16(&pal_bg_bank[0], bg_bootscreenPal, bg_bootscreenPalLen/2);

    REG_BG0CNT = BG_PRIO(3) | BG_CBB(0) | BG_SBB(31);
    REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_BG2;

    irq_init(NULL);
    irq_add(II_VBLANK, NULL);

    SLEEP(3);
    fadeout(3, 2, BLD_BG0 | BLD_BG2);

    clear_tile_mem();

    GRIT_CPY32(&tile_mem[0][0], bg_bootscreen2Tiles);
    GRIT_CPY32(&se_mem[31][0], bg_bootscreen2Map);
    GRIT_CPY16(&pal_bg_bank[0], bg_bootscreen2Pal);

    tte_init_se_default(2, BG_CBB(2) | BG_SBB(28));
    tte_init_con();

    tte_printf("HELLO WORLD\n\n\n");

    SLEEP(3);
    fadein(3, 2, BLD_BG0 | BLD_BG2);

    while(TRUE)
    {
        VBlankIntrWait();

        frame++;
    }
    return 0;
}

// speed in frames
// multiple bgs are possible
void fadeout(int mode, int speed, int bg)
{
    REG_BLDCNT = BLD_BUILD(
        bg,
        0,
        mode);

    for (int i = 1; i < 0x11; i++)
    {
        VBlankIntrDelay(speed);
        REG_BLDY = BLDY_BUILD(i);
    }
}

// Generate fade table and return pointer to memory
// Do not generate your own fade tables, use the given functions
// !!!!REFACTOR BECAUSE VERY HARDCODED
COLOR *fadeout_bg(int start, int end)
{
    int size = (start + end) + 1;
    COLOR temp_pal[size];
    COLOR *temp_fade_tbl = malloc(256*32);
    COLOR clr = 0;
    int r = 0, g = 0, b = 0;

    memcpy16(temp_pal, pal_bg_mem, size);

    for (int counter = 0; counter < 32; counter++)
    {
        VBlankIntrDelay(1);
        memcpy16(&temp_fade_tbl[counter * 256], temp_pal, size);
        for (int i = 0; i < size; i++)
        {
            clr = temp_pal[i];
            r = get_clr_chn(clr, 0);
            g = get_clr_chn(clr, 1);
            b = get_clr_chn(clr, 2);

            r = clamp(--r, 0, 32);
            g = clamp(--g, 0, 32);
            b = clamp(--b, 0, 32);

            clr = r | g << 5 | b << 10;
            temp_pal[i] = clr;
        }

        memcpy16(pal_bg_mem, temp_pal, size);
    }

    return temp_fade_tbl;
}

// !!!! REFACTOR BECAUSE VERY HARDCODED
void fadein_bg(COLOR *fade_tbl)
{
    for (int i = 31; i >= 0; i--)
    {
        VBlankIntrDelay(2);
        memcpy16(pal_bg_mem, &fade_tbl[i * 256], 256);
    }
}

void fadein(int mode, int speed, int bg)
{
    REG_BLDCNT = BLD_BUILD(
        bg,
        0,
        mode);

    for (int i = 0x11; i > 0; i--)
    {
        VBlankIntrDelay(speed);
        REG_BLDY = BLDY_BUILD(i);
    }
}

// r = 0, g = 1, b = 2
INLINE int get_clr_chn(COLOR clr, int n)
{
    int chn = bf_get(clr, n * 5, 5);
    return chn;
}

INLINE void clear_pals()
{
    memset16(pal_bg_mem, 0, 128 * (sizeof *pal_bg_mem));
}

INLINE void clear_tile_mem()
{
    memset32(tile_mem, 0, 6 * sizeof (*tile_mem));
}

//if (bld_var != 0x10 && (frame & 1) == 0)
  //          {
    //            bld_var++;
      //      }
