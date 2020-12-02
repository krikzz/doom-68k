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
//	Fixed point implementation.
//
//-----------------------------------------------------------------------------


static const char
rcsid[] = "$Id: m_bbox.c,v 1.1 1997/02/03 22:45:10 b1 Exp $";

#include "sys.h"

#include "doomtype.h"
#include "i_system.h"

#ifdef __GNUG__
#pragma implementation "m_fixed.h"
#endif
#include "m_fixed.h"




// Fixme. __USE_C_FIXED__ or something.

fixed_t FixedMul(fixed_t a, fixed_t b) {

    REG_MUL_A = a;
    REG_MUL_B = b;
    return REG_MUL_F;
    //return ((long long) a * (long long) b) >> FRACBITS;
}



//
// FixedDiv, C version.
//

fixed_t FixedDiv(fixed_t a, fixed_t b) {

    /*
    if ((std_abs(a) >> 14) >= std_abs(b)) {
        return (a^b) < 0 ? MININT : MAXINT;
    }*/

    //return ((long long) a << 16) / b;
    REG_DIV_A = a;
    REG_DIV_B = b;
    return REG_DIV_R;

    //return FixedDiv2(a, b);
}

/*
fixed_t FixedDiv2(fixed_t a, fixed_t b) {

#if 0
    long long c;
    c = ((long long) a << 16) / ((long long) b);
    return (fixed_t) c;
#endif

    double c;

    c = ((double) a) / ((double) b) * FRACUNIT;

  
    return (fixed_t) c;
}
 */
