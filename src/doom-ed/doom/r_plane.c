// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	Here is a core component: drawing the floors and ceilings,
//	 while maintaining a per column clipping list only.
//	Moreover, the sky areas have to be determined.
//
//-----------------------------------------------------------------------------


static const char
rcsid[] = "$Id: r_plane.c,v 1.4 1997/02/03 16:47:55 b1 Exp $";


#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"

#include "doomdef.h"
#include "doomstat.h"

#include "r_local.h"
#include "r_sky.h"
#include "bios.h"



planefunction_t floorfunc;
planefunction_t ceilingfunc;

//
// opening
//

// Here comes the obnoxious "visplane".
#define MAXVISPLANES	128
visplane_t visplanes[MAXVISPLANES];
visplane_t* lastvisplane;
visplane_t* floorplane;
visplane_t* ceilingplane;

// ?
#define MAXOPENINGS	SCREENWIDTH*64
short openings[MAXOPENINGS];
short* lastopening;


//
// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
//
short floorclip[SCREENWIDTH];
short ceilingclip[SCREENWIDTH];

//
// spanstart holds the start of a plane span
// initialized to 0 at start
//
int spanstart[SCREENHEIGHT];
int spanstop[SCREENHEIGHT];

//
// texture mapping
//
lighttable_t** planezlight;
fixed_t planeheight;

fixed_t yslope[SCREENHEIGHT];
fixed_t distscale[SCREENWIDTH];
fixed_t basexscale;
fixed_t baseyscale;

fixed_t cachedheight[SCREENHEIGHT];
fixed_t cacheddistance[SCREENHEIGHT];
fixed_t cachedxstep[SCREENHEIGHT];
fixed_t cachedystep[SCREENHEIGHT];



//
// R_InitPlanes
// Only at game startup.
//

void R_InitPlanes(void) {
    // Doh!
}


//
// R_MapPlane
//
// Uses global vars:
//  planeheight
//  ds_source
//  basexscale
//  baseyscale
//  viewx
//  viewy
//
// BASIC PRIMITIVE
//




//
// R_ClearPlanes
// At begining of frame.
//

void R_ClearPlanes(void) {
    int i;
    angle_t angle;

    // opening / clipping determination
    for (i = 0; i < viewwidth; i++) {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    lastvisplane = visplanes;
    lastopening = openings;

    // texture calculation
    std_memset(cachedheight, 0, sizeof (cachedheight));

    // left to right mapping
    angle = (viewangle - ANG90) >> ANGLETOFINESHIFT;

    // scale will be unit scale at SCREENWIDTH/2 distance
    basexscale = FixedDiv(finecosine[angle], centerxfrac);
    baseyscale = -FixedDiv(finesine[angle], centerxfrac);
}




//
// R_FindPlane
//

visplane_t* R_FindPlane(fixed_t height, int picnum, int lightlevel) {

    visplane_t* check;

    if (picnum == skyflatnum) {
        height = 0; // all skys map together
        lightlevel = 0;
    }

    for (check = visplanes; check < lastvisplane; check++) {
        
        if (height == check->height
                && picnum == check->picnum
                && lightlevel == check->lightlevel) {
            break;
        }
    }


    if (check < lastvisplane)return check;


 
    lastvisplane++;

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->minx = SCREENWIDTH;
    check->maxx = -1;

    //std_memset(check->top, 0xff, sizeof (check->top));
    std_memset(check->top, 0xff, sizeof (check->top));
    
    //bi_cmd_mem_set(0xff, (u32)check->top, sizeof (check->top));

    return check;
}


//
// R_CheckPlane
//

visplane_t*
R_CheckPlane
(visplane_t* pl,
        int start,
        int stop) {
    int intrl;
    int intrh;
    int unionl;
    int unionh;
    int x;

    if (start < pl->minx) {
        intrl = pl->minx;
        unionl = start;
    } else {
        unionl = pl->minx;
        intrl = start;
    }

    if (stop > pl->maxx) {
        intrh = pl->maxx;
        unionh = stop;
    } else {
        unionh = pl->maxx;
        intrh = stop;
    }

    for (x = intrl; x <= intrh; x++)
        if (pl->top[x] != 0xff)
            break;

    if (x > intrh) {
        pl->minx = unionl;
        pl->maxx = unionh;

        // use the same one
        return pl;
    }

    // make a new visplane
    lastvisplane->height = pl->height;
    lastvisplane->picnum = pl->picnum;
    lastvisplane->lightlevel = pl->lightlevel;

    pl = lastvisplane++;
    pl->minx = start;
    pl->maxx = stop;

    std_memset(pl->top, 0xff, sizeof (pl->top));

    return pl;
}

inline fixed_t FixedMul_rp(fixed_t a, fixed_t b) {

    REG_MUL_A = a;
    REG_MUL_B = b;
    return REG_MUL_F;
    //return ((long long) a * (long long) b) >> FRACBITS;
}

inline void R_DrawSpan_in(void) {

    byte* dest;
    s16 count;
    extern byte * ylookup[];
    extern int columnofs[];


    //ds_source configured in R_MakeSpans
    REG_XFRAC = ds_xfrac;
    REG_YFRAC = ds_yfrac;
    REG_XSTEP = ds_xstep;
    REG_YSTEP = ds_ystep;

    dest = ylookup[ds_y] + columnofs[ds_x1];
    count = ds_x2 - ds_x1;

    do {
        *dest++ = REG_SPOT; //ds_source[REG_SPOT];
    } while (count--);
}

//int cott;

void R_MapPlane_in(int y, int x2) {

    angle_t angle;
    fixed_t distance;
    fixed_t length;
    int x1;

    x1 = spanstart[y];

    //cache was removed
    distance = FixedMul_rp(planeheight, yslope[y]);
    REG_XSTEP = FixedMul_rp(distance, basexscale);
    REG_YSTEP = FixedMul_rp(distance, baseyscale);
    length = FixedMul_rp(distance, distscale[x1]);

    angle = (viewangle + xtoviewangle[x1]) >> ANGLETOFINESHIFT;

    REG_XFRAC = viewx + FixedMul_rp(finecosine[angle], length);
    REG_YFRAC = -viewy - FixedMul_rp(finesine[angle], length);

    ds_colormap = fixedcolormap;

    //R_DrawSpan_in();
    extern byte * ylookup[];
    extern int columnofs[];

    byte *dest = ylookup[y] + columnofs[x1];
    u16 count = x2 - x1;
    //cott ++;


    while (count > 16) {
        *dest++ = REG_SPOT;
        *dest++ = REG_SPOT;
        *dest++ = REG_SPOT;
        *dest++ = REG_SPOT;
        *dest++ = REG_SPOT;
        *dest++ = REG_SPOT;
        *dest++ = REG_SPOT;
        *dest++ = REG_SPOT;
        *dest++ = REG_SPOT;
        *dest++ = REG_SPOT;
        *dest++ = REG_SPOT;
        *dest++ = REG_SPOT;
        *dest++ = REG_SPOT;
        *dest++ = REG_SPOT;
        *dest++ = REG_SPOT;
        *dest++ = REG_SPOT;
        count -= 16;
    }

    do {
        *dest++ = REG_SPOT; //ds_source[REG_SPOT]; (configured in R_MakeSpans)
    } while (count--);
}

//
// R_MakeSpans
//

void R_MakeSpans(int x, int t1, int b1, int t2, int b2) {


    //REG_FBUFF = (u32) ds_source; //used in R_MapPlane

    while (t1 < t2 && t1 <= b1) {
        R_MapPlane_in(t1, x - 1);
        t1++;
    }

    while (b1 > b2 && b1 >= t1) {
        R_MapPlane_in(b1, x - 1);
        b1--;
    }

    while (t2 < t1 && t2 <= b2) {
        spanstart[t2] = x;
        t2++;
    }

    while (b2 > b1 && b2 >= t2) {
        spanstart[b2] = x;
        b2--;
    }
}



//
// R_DrawPlanes
// At the end of each frame.
//

void R_DrawPlanes(void) {

    visplane_t* pl;
    int light;
    int x;
    int stop;
    int angle;



    for (pl = visplanes; pl < lastvisplane; pl++) {


        if (pl->minx > pl->maxx)continue;

        // sky flat
        if (pl->picnum == skyflatnum) {

            dc_iscale = pspriteiscale >> detailshift;

            // Sky is allways drawn full bright,
            //  i.e. colormaps[0] is used.
            // Because of this hack, sky is not affected
            //  by INVUL inverse mapping.
            dc_colormap = colormaps;
            dc_texturemid = skytexturemid;
            
            for (x = pl->minx; x <= pl->maxx; x++) {
                
                dc_yl = pl->top[x];
                dc_yh = pl->bottom[x];

                if (dc_yl <= dc_yh) {
                    angle = (viewangle + xtoviewangle[x]) >> ANGLETOSKYSHIFT;
                    dc_x = x;
                    dc_source = R_GetColumn(skytexture, angle);
                    colfunc();
                }
            }
            continue;
        }


        // regular flat
        //ds_source = W_CacheLumpNum(firstflat + flattranslation[pl->picnum], PU_STATIC);
        REG_FBUFF = (u32) W_CacheLumpNum(firstflat + flattranslation[pl->picnum], PU_STATIC); //used in R_MapPlane

        //planeheight = std_abs(pl->height - viewz); 
        planeheight = pl->height - viewz;
        if (planeheight < 0)planeheight = -planeheight;
        light = (pl->lightlevel >> LIGHTSEGSHIFT) + extralight;

        if (light >= LIGHTLEVELS)light = LIGHTLEVELS - 1;

        if (light < 0)light = 0;

        planezlight = zlight[light];

        pl->top[pl->maxx + 1] = 0xff;
        pl->top[pl->minx - 1] = 0xff;

        stop = pl->maxx + 1;


        for (x = pl->minx; x <= stop; x++) {

            R_MakeSpans(x, pl->top[x - 1],
                    pl->bottom[x - 1],
                    pl->top[x],
                    pl->bottom[x]);
        }

        //Z_ChangeTag(ds_source, PU_CACHE);

    }

    //std_printf("count: %d\n", cott);

}
