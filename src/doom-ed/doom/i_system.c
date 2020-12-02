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
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: m_bbox.c,v 1.1 1997/02/03 22:45:10 b1 Exp $";

#include "sys.h"

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"

#include "d_net.h"
#include "g_game.h"

#ifdef __GNUG__
#pragma implementation "i_system.h"
#endif
#include "i_system.h"




int mb_used = 3;

void I_Tactile(int on, int off, int total) {

    // UNUSED.
    on = off = total = 0;
}

ticcmd_t emptycmd;

ticcmd_t* I_BaseTiccmd(void) {
    return &emptycmd;
}

int I_GetHeapSize(void) {
    return SIZE_DOOM_RAM;
}

byte* I_ZoneBase(int* size) {
    *size = I_GetHeapSize();
    return (byte *) std_malloc(*size);
}



//
// I_GetTime
// returns time in 1/70th second tics
//

int I_GetTime(void) {

    return sysGetTime();

}



//
// I_Init
//

void I_Init(void) {
    I_InitSound();
    //  I_InitGraphics();
}

//
// I_Quit
//

void I_Quit(void) {
    D_QuitNetGame();
    I_ShutdownSound();
    I_ShutdownMusic();
    M_SaveDefaults();
    I_ShutdownGraphics();
    std_exit(0);
}

void I_WaitVBL(int count) {
#ifdef SGI
    sginap(1);
#else
#ifdef SUN
    sleep(0);
#else
    //sys_sleep(count * (1000000 / 70));
    std_vsync();
#endif
#endif
}

void I_BeginRead(void) {
}

void I_EndRead(void) {
}

byte* I_AllocLow(int length) {
    byte* mem;

    mem = (byte *) std_malloc(length);
    std_memset(mem, 0, length);
    return mem;
}


//
// I_Error
//
extern doomboolean demorecording;

void I_Error(char *error, ...) {


    //va_start(argptr, error);
    std_printf("\nERR:");
    std_printf(error);

    /*
    va_list argptr;

    // Message first.
    va_start(argptr, error);
    sys_fprintf(stderr, "Error: ");
    vfprintf(stderr, error, argptr);
    //sys_fprintf(stderr, "\n");
    va_end(argptr);

    //fflush( stderr );

    // Shutdown. Here might be other errors.
    if (demorecording)
        G_CheckDemoStatus();

    D_QuitNetGame();
    I_ShutdownGraphics();*/

    std_exit(-1);
}
