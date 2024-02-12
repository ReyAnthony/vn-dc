#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

#include "SDL/SDL.h"
#include "../text/text.h"

typedef enum directions_e {
    UP,
    DOWN
} MENU_Directions;

typedef enum placement_mode_e {
    CENTERED,
    LEFT_ALIGNED,
    RIGHT_ALIGNED
} MENU_Placement_Mode;

typedef struct bounds_t {
    //Uint16 width;
    //Uint16 height;
    Uint16 x;
    Uint16 y;
} MENU_Bounds;

typedef struct menu_def_data_t {
    char name[128];
    void (*on_select)();
} MENU_Definition_data;

typedef struct defintition_t {

    MENU_Definition_data menus[32];
    Uint8 menu_count;

    MENU_Bounds bounds;
    MENU_Placement_Mode placement_mode;
    Uint8 wrap;
    char* bkg_file;

} MENU_Definition;

int MENU_init(MENU_Definition menu_def);
void MENU_move(MENU_Directions direction);
void MENU_select();
void MENU_draw(Uint32 delta_time);
void MENU_quit();

#endif // MENU_H_INCLUDED
