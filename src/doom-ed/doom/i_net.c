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


#include "i_system.h"
#include "d_event.h"
#include "d_net.h"
#include "m_argv.h"

#include "doomstat.h"

#ifdef __GNUG__
#pragma implementation "i_net.h"
#endif
#include "i_net.h"



//
// NETWORKING
//

int DOOMPORT = 0; //(IPPORT_USERRESERVED +0x1d );

int sendsocket;
int insocket;

//struct sockaddr_in sendaddress[MAXNETNODES];

void (*netget) (void);
void (*netsend) (void);

void I_InitNetwork(void) {

    doomcom = std_malloc(sizeof (*doomcom));
    std_memset(doomcom, 0, sizeof (*doomcom));

    doomcom->ticdup = 1;
    doomcom->extratics = 0;
    netgame = false;
    doomcom->id = DOOMCOM_ID;
    doomcom->numplayers = doomcom->numnodes = 1;
    doomcom->deathmatch = false;
    doomcom->consoleplayer = 0;

    /* 
    int i;
    int p;
    // set up for network
    i = M_CheckParm("-dup");

    if (i && i < myargc - 1) {
        doomcom->ticdup = myargv[i + 1][0] - '0';
        if (doomcom->ticdup < 1)
            doomcom->ticdup = 1;
        if (doomcom->ticdup > 9)
            doomcom->ticdup = 9;
    } else {
        doomcom-> ticdup = 1;
    }

    if (M_CheckParm("-extratic"))
        doomcom-> extratics = 1;
    else
        doomcom-> extratics = 0;

    p = M_CheckParm("-port");
    if (p && p < myargc - 1) {
        DOOMPORT = sys_atoi(myargv[p + 1]);
        sys_printf("using alternate port %i\n", DOOMPORT);
    }

    // parse network game options,
    //  -net <consoleplayer> <host> <host> ...
    i = M_CheckParm("-net");
    if (!i) {
        // single player game
        netgame = false;
        doomcom->id = DOOMCOM_ID;
        doomcom->numplayers = doomcom->numnodes = 1;
        doomcom->deathmatch = false;
        doomcom->consoleplayer = 0;
        return;
    }*/

}

void I_NetCmd(void) {
    if (doomcom->command == CMD_SEND) {
        netsend();
    } else if (doomcom->command == CMD_GET) {
        netget();
    } else
        I_Error("Bad net cmd: %i\n", doomcom->command);
}

