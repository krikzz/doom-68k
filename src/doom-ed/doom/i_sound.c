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
//	System interface for sound.
//
//-----------------------------------------------------------------------------

static const char rcsid[] = "$Id: i_unix.c,v 1.5 1997/02/03 22:45:10 b1 Exp $";


#include "z_zone.h"

#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"

#include "doomdef.h"

//
// Safe ioctl, convenience.
//

void myioctl(int fd, int command, int* arg) {

}


//
// This function loads the sound data from the WAD lump,
//  for single sound.
//

void* getsfx(char* sfxname, int* len) {

    sfxname = 0;
    len = 0;

    return 0;
}


//
// This function adds a sound to the
//  list of currently active sounds,
//  which is maintained as a given number
//  (eight, usually) of internal channels.
// Returns a handle.
//

int addsfx(int sfxid, int volume, int step, int seperation) {

    sfxid = volume = step = seperation = 0;

    return 0;
}



//
// SFX API
// Note: this was called by S_Init.
// However, whatever they did in the
// old DPMS based DOS version, this
// were simply dummies in the Linux
// version.
// See soundserver initdata().
//

void I_SetChannels() {
    // Init internal lookups (raw data, mixing buffer, channels).
    // This function sets up internal lookups used during
    //  the mixing process. 
}

void I_SetSfxVolume(int volume) {
    // Identical to DOS.
    // Basically, this should propagate
    //  the menu/config file setting
    //  to the state variable used in
    //  the mixing.
    volume = 0;
}

// MUSIC API - dummy. Some code from DOS version.

void I_SetMusicVolume(int volume) {
    // Internal state variable.
    // Now set volume on output device.
    // Whatever( snd_MusciVolume );
    volume = 0;
}


//
// Retrieve the raw data lump index
//  for a given SFX name.
//

int I_GetSfxLumpNum(sfxinfo_t* sfx) {
    sfx = 0;
    return 0;
}

//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//

int I_StartSound(int id, int vol, int sep, int pitch, int priority) {
    
    id = vol = sep = pitch = priority = 0;

    return 0;
}

void I_StopSound(int handle) {
    // You need the handle returned by StartSound.
    // Would be looping all channels,
    //  tracking down the handle,
    //  an setting the channel to zero.

    // UNUSED.
    handle = 0;
}

int I_SoundIsPlaying(int handle) {
    // Ouch.
    return gametic < handle;
}




//
// This function loops all active (internal) sound
//  channels, retrieves a given number of samples
//  from the raw sound data, modifies it according
//  to the current (internal) channel parameters,
//  mixes the per channel samples into the global
//  mixbuffer, clamping it to the allowed range,
//  and sets up everything for transferring the
//  contents of the mixbuffer to the (two)
//  hardware channels (left and right, that is).
//
// This function currently supports only 16bit.
//

void I_UpdateSound(void) {

}


// 
// This would be used to write out the mixbuffer
//  during each game loop update.
// Updates sound buffer and audio device at runtime. 
// It is called during Timer interrupt with SNDINTR.
// Mixing now done synchronous, and
//  only output be done asynchronous?
//

void I_SubmitSound(void) {
}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch) {

    // I fail too see that this is used.
    // Would be using the handle to identify
    //  on which channel the sound might be active,
    //  and resetting the channel parameters.

    // UNUSED.
    handle = vol = sep = pitch = 0;
}

void I_ShutdownSound(void) {

}

void I_InitSound() {
}




//
// MUSIC API.
// Still no music done.
// Remains. Dummies.
//

void I_InitMusic(void) {
}

void I_ShutdownMusic(void) {
}

static int looping = 0;
static int musicdies = -1;

void I_PlaySong(int handle, int looping) {

    handle = looping = 0;
}

void I_PauseSong(int handle) {
    // UNUSED.
    handle = 0;
}

void I_ResumeSong(int handle) {
    // UNUSED.
    handle = 0;
}

void I_StopSong(int handle) {
    // UNUSED.
    handle = 0;

    looping = 0;
    musicdies = 0;
}

void I_UnRegisterSong(int handle) {
    // UNUSED.
    handle = 0;
}

int I_RegisterSong(void* data) {
    // UNUSED.
    data = 0;

    return 1;
}

// Is the song playing?

int I_QrySongPlaying(int handle) {
    // UNUSED.
    handle = 0;
    return looping || musicdies > gametic;
}



//
// Experimental stuff.
// A Linux timer interrupt, for asynchronous
//  sound output.
// I ripped this out of the Timer class in
//  our Difference Engine, including a few
//  SUN remains...
//  



// Interrupt handler.

void I_HandleSoundTimer(int ignore) {

    ignore = 0;

    return;
}

// Get the interrupt. Set duration in millisecs.

int I_SoundSetTimer(int duration_of_tick) {

    duration_of_tick = 0;

    return 0;
}


// Remove the interrupt. Set duration to zero.

void I_SoundDelTimer() {

}
