#ifndef TEXT_H_INCLUDED
#define TEXT_H_INCLUDED

#include <stdint.h>
#include "../gfx/gfx.h"

typedef struct {
    int x; int y;
} TEXT_Point;

typedef struct {
    int w; int h;
} TEXT_Rect;

#define TEXT_Point_default (TEXT_Point) {.x = 0, .y = 0}
#define TEXT_Point_init_pos(xx, yy) (TEXT_Point) {.x = xx, .y = yy}

int TEXT_init(char* charset, uint8_t char_height, uint8_t char_width);

void TEXT_blit(TEXT_Point* pos, char* text);
void TEXT_blit_formatted(TEXT_Point* pos, const char* format, ...);
TEXT_Rect TEXT_precal_rect(char* text);

void TEXT_quit();

#endif // TEXT_H_INCLUDED
