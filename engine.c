#include "engine.h"

#if DC
#include <sdl/sdl.h>
#include <sdl/sdl_events.h>
#endif // DC

#define FAILURE 0
#define SUCCESS 1
#define BUTTON_REPEAT_DELAY_IN_SEC 0.150f

//https://phabricator.kairohm.dev/file/data/4d4zryheuoomxjxw6w62/PHID-FILE-2yzob5psz5ob4lallyd6/file
//how to setup an assert handler (for example for a custom crash screen)
//TODO check this for easing debug

typedef struct Timer_s {
   Uint32 previous_ticks;
   Uint32 current_ticks;
   float delta_time;
} Timer;

static void update_timer();
static void init_timer();
static void draw();
static void update(SDL_Event* event);

static Timer timer;
static float repeat_time = 0;
static Uint8 running = 0;
static SDL_Event event;


typedef struct RegisteredSystems {
    Engine_SubsystemBind systems[16];
    Uint8 registered_systems_count;

} RegisteredSystems;

static RegisteredSystems systems;

int ENGINE_init() {

    hlp_log_ln(HLP_LOG_INFO, "Initialising ENGINE.");
    setbuf(stdout, NULL); //just spit everything asap.

    //todo add an option to log to the framebuffer (fb) but not display anything
    //to be able to check on DC without cable
    #ifdef DC
    #if !NO_LOGGING
    //logging to the // port
    if(dbgio_dev_select("scif") == -1) {

       hlp_log_ln(HLP_LOG_ERROR, "Could not init debug io to scif. Rebuild the game without this (or connect a coder cable)");
       hlp_log_ln(HLP_LOG_ERROR, "Will keep running, but logging will most likely get lost on real hardware.");
    };
    #endif
    #endif // DC

    #if DC
    //not initializing video do not start the event system pff
    //We won't create the SDL graphic context on the Dreamcast anyway.
    //PC will use keyboard and do not needs this.
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) != 0) {

        hlp_log_ln(HLP_LOG_ERROR, "Could not init SDL.");
        return 1;
    }

    SDL_ShowCursor(0);
    SDL_JoystickOpen(0);
    #endif // DC

    srand(time(NULL));
    init_timer();

    if(GFX_init() != 0) {

        hlp_log_ln(HLP_LOG_ERROR, "Failed to init GFX, aborting.");
        return FAILURE;
    }

    if(!TEXT_init(FONTS_PATH, FONT_H, FONT_W)) {

        hlp_log_ln(HLP_LOG_ERROR, "Could not init TEXT engine.");
        return FAILURE;
    }

    if(!AUDIO_init()) {

        hlp_log_ln(HLP_LOG_ERROR, "Could not init AUDIO engine.");
        return FAILURE;
    }

    systems.registered_systems_count = 0;
    return SUCCESS;
}

void ENGINE_quit() {

    hlp_log_ln(HLP_LOG_INFO, "ENGINE is Quitting !");

    //quit + unbind subsystems
    while(systems.registered_systems_count > 0) {

        ENGINE_subsystem_unbind(systems.registered_systems_count -1);
    }

    AUDIO_quit();
    TEXT_quit();
    GFX_quit();

    #if DC
    SDL_Quit();
    #endif // DC

    hlp_log_ln(HLP_LOG_INFO, "Engine has quitted !");
}

void ENGINE_start() {

    running = 1;
    while(running) {
        update_timer();
        update(&event);
        draw();
    }

    hlp_log_ln(HLP_LOG_INFO, "Ok, gotcha, main loop exited, will quit and clean the game !");
}

void ENGINE_stop() {

    hlp_log_ln(HLP_LOG_INFO, "Explicit engine stop was requested");
    running = 0;
}

int ENGINE_subsystem_bind(Engine_SubsystemBind subsystem_binding) {

    hlp_log_ln(HLP_LOG_INFO, "Binding a subsystem (id %d)", systems.registered_systems_count);

    systems.systems[systems.registered_systems_count] = subsystem_binding;
    systems.systems[systems.registered_systems_count].init();
    systems.registered_systems_count++;
    return systems.registered_systems_count - 1;
}

//TODO TODO TODO
//will not work when multiple subsystems will be used, as it will break the subsystems IDs,
//need to pass a pointer or something.
void ENGINE_subsystem_unbind(int id) {

    hlp_log_ln(HLP_LOG_INFO, "Unbinding a subsystem (id %d)", id);
    systems.systems[id].quit();

    systems.systems[id].init = NULL;
    systems.systems[id].draw = NULL;
    systems.systems[id].quit = NULL;
    systems.systems[id].update = NULL;

    systems.registered_systems_count--;

    //shift the others
    for (int i = id; i <  systems.registered_systems_count - 1; i++) {
        systems.systems[i] = systems.systems[i + 1];
    }
}

// STATIC FUNCTIONS
static void update_timer() {

    timer.previous_ticks = timer.current_ticks;
    timer.current_ticks = SDL_GetTicks();

    //DT is in seconds
    timer.delta_time = (float)((timer.current_ticks - timer.previous_ticks) * 0.001f); //mult is faster than div
}

static void init_timer() {

     timer.current_ticks = SDL_GetTicks();
}

static void draw() {

    GFX_start_draw();

   //draw susbsystems
    for (int i = 0; i < systems.registered_systems_count; i++) {
        systems.systems[i].draw(timer.delta_time);
    }

    GFX_end_draw();
}

static void update(SDL_Event* ev){

    repeat_time += timer.delta_time;
    SDL_PollEvent(ev);

    //TODO we should accept the first input at once, but delay the repeat
    #ifdef PC
    if (repeat_time > BUTTON_REPEAT_DELAY_IN_SEC) {
         if (ev->type == SDL_KEYDOWN && repeat_time > BUTTON_REPEAT_DELAY_IN_SEC) {
            repeat_time = 0;
        }
        if(ev->type == SDL_KEYDOWN && ev->key.keysym.sym == SDLK_ESCAPE) {
              ENGINE_stop();
        }

        for (int i = 0; i < systems.registered_systems_count; i++) {
            systems.systems[i].handle_input(ev, timer.delta_time);
        }
    }
    #endif
    #ifdef DC
    if (repeat_time > BUTTON_REPEAT_DELAY_IN_SEC) {
         if ((ev->type == SDL_JOYBUTTONDOWN || ev->type == SDL_JOYHATMOTION) && repeat_time > BUTTON_REPEAT_DELAY_IN_SEC) {
            repeat_time = 0;
        }

        for (int i = 0; i < systems.registered_systems_count; i++) {
            systems.systems[i].handle_input(ev, timer.delta_time);
        }
    }
    #endif

    for (int i = 0; i < systems.registered_systems_count; i++) {
        systems.systems[i].update(timer.delta_time);
    }
}
