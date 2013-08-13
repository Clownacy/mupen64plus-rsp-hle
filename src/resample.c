/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-rsp-hle - resample.c                                      *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2012 Bobby Smiles                                       *
 *   Copyright (C) 2009 Richard Goedeken                                   *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdbool.h>
#include <stdint.h>

#include "hle.h"
#include "arithmetic.h"
#include "memory.h"

static const uint16_t RESAMPLE_LUT [4 * 64] =
{
    0x0c39, 0x66ad, 0x0d46, 0xffdf, 0x0b39, 0x6696, 0x0e5f, 0xffd8,
    0x0a44, 0x6669, 0x0f83, 0xffd0, 0x095a, 0x6626, 0x10b4, 0xffc8,
    0x087d, 0x65cd, 0x11f0, 0xffbf, 0x07ab, 0x655e, 0x1338, 0xffb6,
    0x06e4, 0x64d9, 0x148c, 0xffac, 0x0628, 0x643f, 0x15eb, 0xffa1,
    0x0577, 0x638f, 0x1756, 0xff96, 0x04d1, 0x62cb, 0x18cb, 0xff8a,
    0x0435, 0x61f3, 0x1a4c, 0xff7e, 0x03a4, 0x6106, 0x1bd7, 0xff71,
    0x031c, 0x6007, 0x1d6c, 0xff64, 0x029f, 0x5ef5, 0x1f0b, 0xff56,
    0x022a, 0x5dd0, 0x20b3, 0xff48, 0x01be, 0x5c9a, 0x2264, 0xff3a,
    0x015b, 0x5b53, 0x241e, 0xff2c, 0x0101, 0x59fc, 0x25e0, 0xff1e,
    0x00ae, 0x5896, 0x27a9, 0xff10, 0x0063, 0x5720, 0x297a, 0xff02,
    0x001f, 0x559d, 0x2b50, 0xfef4, 0xffe2, 0x540d, 0x2d2c, 0xfee8,
    0xffac, 0x5270, 0x2f0d, 0xfedb, 0xff7c, 0x50c7, 0x30f3, 0xfed0,
    0xff53, 0x4f14, 0x32dc, 0xfec6, 0xff2e, 0x4d57, 0x34c8, 0xfebd,
    0xff0f, 0x4b91, 0x36b6, 0xfeb6, 0xfef5, 0x49c2, 0x38a5, 0xfeb0,
    0xfedf, 0x47ed, 0x3a95, 0xfeac, 0xfece, 0x4611, 0x3c85, 0xfeab,
    0xfec0, 0x4430, 0x3e74, 0xfeac, 0xfeb6, 0x424a, 0x4060, 0xfeaf,
    0xfeaf, 0x4060, 0x424a, 0xfeb6, 0xfeac, 0x3e74, 0x4430, 0xfec0,
    0xfeab, 0x3c85, 0x4611, 0xfece, 0xfeac, 0x3a95, 0x47ed, 0xfedf,
    0xfeb0, 0x38a5, 0x49c2, 0xfef5, 0xfeb6, 0x36b6, 0x4b91, 0xff0f,
    0xfebd, 0x34c8, 0x4d57, 0xff2e, 0xfec6, 0x32dc, 0x4f14, 0xff53,
    0xfed0, 0x30f3, 0x50c7, 0xff7c, 0xfedb, 0x2f0d, 0x5270, 0xffac,
    0xfee8, 0x2d2c, 0x540d, 0xffe2, 0xfef4, 0x2b50, 0x559d, 0x001f,
    0xff02, 0x297a, 0x5720, 0x0063, 0xff10, 0x27a9, 0x5896, 0x00ae,
    0xff1e, 0x25e0, 0x59fc, 0x0101, 0xff2c, 0x241e, 0x5b53, 0x015b,
    0xff3a, 0x2264, 0x5c9a, 0x01be, 0xff48, 0x20b3, 0x5dd0, 0x022a,
    0xff56, 0x1f0b, 0x5ef5, 0x029f, 0xff64, 0x1d6c, 0x6007, 0x031c,
    0xff71, 0x1bd7, 0x6106, 0x03a4, 0xff7e, 0x1a4c, 0x61f3, 0x0435,
    0xff8a, 0x18cb, 0x62cb, 0x04d1, 0xff96, 0x1756, 0x638f, 0x0577,
    0xffa1, 0x15eb, 0x643f, 0x0628, 0xffac, 0x148c, 0x64d9, 0x06e4,
    0xffb6, 0x1338, 0x655e, 0x07ab, 0xffbf, 0x11f0, 0x65cd, 0x087d,
    0xffc8, 0x10b4, 0x6626, 0x095a, 0xffd0, 0x0f83, 0x6669, 0x0a44,
    0xffd8, 0x0e5f, 0x6696, 0x0b39, 0xffdf, 0x0d46, 0x66ad, 0x0c39
};

/* local functions */
static int32_t dmul(int16_t x, int16_t y)
{
    return ((int32_t)x * (int32_t)y) >> 15;
}

static int16_t dot4(int16_t *x, int16_t *y)
{
    unsigned int i;
    int32_t accu = 0;

    for(i = 0; i < 4; ++i)
        accu  += dmul(x[i], y[i]);

    return clamp_s16(accu);
}

static int16_t* sample(uint16_t i)
{
    return (int16_t*)rsp.DMEM + (i^S);
}

static void reset(uint16_t history_idx, uint32_t *pitch_accu)
{
    unsigned int i;

    for (i = 0; i < 4; ++i)
        *sample(history_idx++) = 0;

    *pitch_accu = 0;
}

static void load(uint16_t history_idx, uint32_t *pitch_accu, uint32_t dram)
{
    unsigned int i;

    dram >>= 1;

    for (i = 0; i < 4; ++i)
        *sample(history_idx++) = ((int16_t*)rsp.RDRAM)[(dram++)^S];

    *pitch_accu = ((uint16_t*)rsp.RDRAM)[dram^S];
}

static void save(uint16_t history_idx, uint32_t *pitch_accu, uint32_t dram)
{
    unsigned int i;

    dram >>= 1;

    for (i = 0; i < 4; ++i)
        ((int16_t*)rsp.RDRAM)[(dram++)^S] = *sample(history_idx++);

    ((uint16_t*)rsp.RDRAM)[dram^S] = *pitch_accu;
}

static int16_t filter4(uint16_t samplei, uint16_t frac)
{
    unsigned int i;
    int16_t x[4];

    for(i = 0; i < 4; ++i)
        x[i] = *sample(samplei++);

    return dot4(x, (int16_t*)RESAMPLE_LUT + ((frac >> 10) << 2));
}


/* global function */
void resample_buffer(
        bool init,
        bool flag2,
        bool flag4,
        uint32_t address,
        uint32_t pitch,      // Q16.16
        uint16_t samplei,
        uint16_t sampleo,
        uint16_t count)
{
    uint32_t pitch_accu;     // Q16.16

    if (flag2)
        DebugMessage(M64MSG_WARNING, "resample - flag2 not handled");

    if (flag4)
        DebugMessage(M64MSG_WARNING, "resample - flag4 not handled");

    samplei -= 4;

    if (init)
        reset(samplei, &pitch_accu);
    else
        load(samplei, &pitch_accu, address);

    while(count != 0)
    {
        *sample(sampleo++) = filter4(samplei, pitch_accu);

        pitch_accu += pitch;
        samplei += (pitch_accu >> 16);
        pitch_accu &= 0xffff;

        --count;
    }

    save(samplei, &pitch_accu, address);
}

/* zero order hold resample */
void resample_zoh(
        uint32_t pitch_accu, // Q16.16
        uint32_t pitch,      // Q16.16
        uint16_t samplei,
        uint16_t sampleo,
        uint16_t count)
{
    while (count != 0)
    {
        *sample(sampleo++) = *sample(samplei);

        pitch_accu += pitch;
        samplei += (pitch_accu >> 16);
        pitch_accu &= 0xffff;

        --count;
    }
}
