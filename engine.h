#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#ifdef PC
#include <stdio.h>
#include <stdlib.h>
#endif

#include "SDL/SDL_events.h" //for events

#include "paths.h"
#include "helpers.h"
#include "path_helpers.h"
#include "constants.h"
#include "audio/audio.h"
#include "text/text.h"
#include "gfx/gfx.h"

int ENGINE_init();
void ENGINE_start();
void ENGINE_stop();
void ENGINE_quit();

typedef struct {

    int  (*init)();
    void (*update)(float delta_time);
    void (*handle_input)(SDL_Event* event, float delta_time);
    void (*draw)(float delta_time);
    void (*quit)();
} Engine_SubsystemBind;

int ENGINE_subsystem_bind(Engine_SubsystemBind subsystem_binding);
void ENGINE_subsystem_unbind(int id);

#endif // ENGINE_H_INCLUDED
