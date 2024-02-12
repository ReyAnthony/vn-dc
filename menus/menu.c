#include "menu.h"

#define FAILURE 0
#define SUCCESS 1
#define MAX_MENUS 32

#include "../path_helpers.h"
#include "../gfx/gfx.h"
#include "../helpers.h"

#if DC
#include "../gfx/arch/gfx_pvr.h"
#endif // DC

#define ARROW_PATH PPC(arrow.png)

//== Internal static vars
typedef struct menu_data_t {
    char text[255];
    void (*on_select)();

} AMenuData;

typedef struct menus_data_t {
    AMenuData menus[MAX_MENUS];
    Uint8 menu_count;
    Uint8 currently_selected;

    MENU_Bounds bounds;
    MENU_Placement_Mode placement_mode;
    Uint8 wrap;
    GFX_TEX_PTR bkg;
} MenusData;

static MenusData data;
static GFX_TEX_PTR selection_arrow;

int MENU_init(MENU_Definition menu_def) {

    hlp_log_ln(HLP_LOG_INFO, "Initialising MENU system");

    if (menu_def.menu_count > MAX_MENUS || menu_def.menu_count < 0) {

        hlp_log_ln(HLP_LOG_ERROR, "Error in menu def.");
        return FAILURE;
    }

    //todo check the MENU_Bounds
    selection_arrow = GFX_allocate(ARROW_PATH, 0, 0);

    if(selection_arrow == NULL) {

        hlp_log_ln(HLP_LOG_ERROR, "Could not load selection arrow : %s, aborting.", ARROW_PATH);
        return FAILURE;
    }

    data.menu_count = menu_def.menu_count;
    data.wrap = menu_def.wrap;
    data.currently_selected = 0;
    data.bounds = menu_def.bounds;
    data.placement_mode = menu_def.placement_mode;

    #if DC
    data.bkg = GFX_allocate__no_png_alpha(menu_def.bkg_file, 0, 0);
    #else
    data.bkg = GFX_allocate(menu_def.bkg_file, 0, 0);
    #endif // DC

    if(data.bkg == NULL) {

        hlp_log_ln(HLP_LOG_ERROR, "Could not load background : %s, aborting.", menu_def.bkg_file);
        return FAILURE;
    }

    for (int i = 0; i < menu_def.menu_count; i++) {

        //todo check we don't have more than 255 chars or allocate mem
        memcpy( data.menus[i].text, menu_def.menus[i].name, sizeof(menu_def.menus[i].name));
        data.menus[i].on_select = menu_def.menus[i].on_select;
    }

    hlp_log_ln(HLP_LOG_INFO, "MENU system initialised");
    return SUCCESS;
}

void MENU_draw(Uint32 delta_time) {

    GFX_blit(data.bkg, NULL, NULL);
    TEXT_Rect precal = TEXT_precal_rect(data.menus[data.currently_selected].text); //cache this !

    if(data.placement_mode == CENTERED)
    {
        GFX_Rect arrow_pos = GFX_Rect_init_pos(data.bounds.x - 32 -precal.w/2, data.bounds.y + (32 * data.currently_selected));
        GFX_blit_s(selection_arrow, &arrow_pos);
    }

    for (int i = 0; i < data.menu_count; i++) {
        if(data.placement_mode == CENTERED)
        {

            TEXT_Rect precal2 = TEXT_precal_rect(data.menus[i].text); //cache this !
            TEXT_Point pos = TEXT_Point_init_pos(data.bounds.x -precal2.w/2, data.bounds.y  +  (32 * i));
            TEXT_blit(&pos, data.menus[i].text);
        }
    }
}

void MENU_move(MENU_Directions direction) {

    if(direction == DOWN) {
        if(data.currently_selected < data.menu_count -1) {
            data.currently_selected++;
        }
        else {
             if(data.wrap == 1) {
                data.currently_selected = 0;
             }
        }
    }
    else if(direction == UP) {
        if(data.currently_selected > 0) {
            data.currently_selected--;
        }
        else {
             if(data.wrap == 1) {
                data.currently_selected =  data.menu_count -1;
             }
        }
    }
}

void MENU_select() {

    if (data.menus[data.currently_selected].on_select != NULL) {
        data.menus[data.currently_selected].on_select();
    }
}

void MENU_quit() {

    hlp_log_ln(HLP_LOG_INFO, "Quitting the MENU system, will cleanup.");

    GFX_free(selection_arrow);
    GFX_free(data.bkg);

    data.menu_count = 0;
    hlp_log_ln(HLP_LOG_INFO, "MENU system has quitted.");
}
