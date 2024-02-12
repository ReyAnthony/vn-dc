#include "game.h"
#include "vn.h"
#include <math.h>
#include <time.h>

#define MENU_BKG PPC(classroom.png)
#define TITLE PPC(title.png)

#include "../gfx/gfx.h"

//=== MENU ====
static int main_menu_init();
static void main_menu_update(float delta_time);
static void main_menu_input_handling(SDL_Event* event, float delta_time);
static void main_menu_draw(float delta_time);
static void main_menu_quit();
static void main_menu_to_gameplay();
static int menu_system_id;

static GFX_TEX_PTR title;
static GFX_Rect title_rect;

typedef enum {

    MAIN_MENU,
    GAME

} GameState;

static GameState currentState;

int GAME_init() {

    hlp_log_ln(HLP_LOG_INFO, "Starting to init GAME.");

    currentState = MAIN_MENU;
    menu_system_id =
            ENGINE_subsystem_bind((Engine_SubsystemBind) {
                .init = main_menu_init,
                .update = main_menu_update,
                .handle_input = main_menu_input_handling,
                .draw = main_menu_draw,
                .quit = main_menu_quit,
              });

    title = GFX_allocate(TITLE, 0, 0);
    title_rect = GFX_Rect_init_pos(WIDTH/2, 120);

    if(title == NULL) {

        hlp_log_ln(HLP_LOG_ERROR, "Could not load %s", TITLE);
        return 0;
    }

    hlp_log_ln(HLP_LOG_INFO, "GAME loaded.");
    return 1;
}

void GAME_quit() {

    //clean game
    //hmmm, no actual need since this is used as a subsystem in the engine, it will take care of freeing the bound systems by using their quit functions.
    //This could be useful if I decide to share resources with another subsystem for the duration of the whole program.
    //But for now... not so much use.
}


//=== MENU ====
static void main_menu_to_gameplay() {

    hlp_log_ln(HLP_LOG_INFO, "Player requested to move to gameplay");

    ENGINE_subsystem_unbind(menu_system_id);
    currentState = GAME;
    //draw a curtain
    //switch to gameplay (use a game state ?)
    VN_Init();
}

static int main_menu_init() {

    hlp_log_ln(HLP_LOG_INFO, "GAME (main menu holder) init startup");

    MENU_Definition def;
    def.bkg_file = MENU_BKG;
    def.menus[0] = (MENU_Definition_data)  { .name = "Jouer", .on_select = main_menu_to_gameplay };
    def.menus[1] = (MENU_Definition_data)  { .name = "Charger" };
    def.menus[2] = (MENU_Definition_data)  { .name = "Options" };
    def.menus[3] = (MENU_Definition_data)  { .name = "Quitter le jeu", .on_select = ENGINE_stop };
    def.menu_count = 4;

    def.wrap = 1;
    def.bounds = (MENU_Bounds) { .x = WIDTH/2, .y = HEIGHT / 2 + 70 };
    def.placement_mode = CENTERED;

    if(!MENU_init(def)) {

        hlp_log_ln(HLP_LOG_ERROR, "Could not init the MENU system, no obvious way to return error code, aborting NOW.");
        exit(1);
    }
    return 1;
}

static void main_menu_update(float delta_time) {

    //update title movement
    title_rect.y = 120 + ((sin(SDL_GetTicks() / 100) * 5) * delta_time * 60);
}

static void main_menu_input_handling(SDL_Event* event, float delta_time) {

#ifdef PC
    if(event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_RETURN) {
        MENU_select();
    }
    else if(event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_DOWN) {
        MENU_move(DOWN);
    }
    else if(event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_UP) {
         MENU_move(UP);
    }
#endif // PC
#ifdef DC
    if(event->type == SDL_JOYBUTTONDOWN && (event->jbutton.button == DC_BUTTON_A || event->jbutton.button == DC_BUTTON_START)) {
        MENU_select();
    }
    else if(event->jhat.hat == DC_HAT_CROSS) {
            switch(event->jhat.value) {
            case DC_HAT_CROSS_DOWN:
                MENU_move(DOWN);
                break;
            case DC_HAT_CROSS_UP:
                MENU_move(UP);
                break;
            default:
                break;

            }
        }
#endif // DC
}


static void main_menu_draw(float delta_time) {

    MENU_draw(delta_time);
    GFX_blit_s(title, &title_rect);
}

static void main_menu_quit() {

    hlp_log_ln(HLP_LOG_INFO, "Quitting the GAME (main menu holder).");
    GFX_free(title);
    MENU_quit();
    hlp_log_ln(HLP_LOG_INFO, "GAME (main menu holder) has quitted.");
}
