#include "text.h"
#include <stdio.h>
#include <stdarg.h>
#include "../helpers.h"

#define FAILURE 0
#define SUCCESS 1
#define MAX_SURFACES 64
#define MAX_CHARSET_RECT 128

typedef struct charset_t {
    uint8_t char_height;
    uint8_t char_width;
    uint8_t width;
    uint8_t height;
    GFX_TEX_PTR characters;
    GFX_Rect rects[MAX_CHARSET_RECT]; //max 128 chars
} Charset;

static Charset loaded_charset;

static void internal_blit(TEXT_Point* pos, char* text);

int TEXT_init(char* charset, uint8_t char_height, uint8_t char_width) {

    hlp_log_ln(HLP_LOG_INFO, "Initliasing TEXT engine.");

    loaded_charset.characters = GFX_allocate(charset, 0, 0);
    if(loaded_charset.characters == NULL) {

        hlp_log_ln(HLP_LOG_ERROR, "Failed to load charset from %s", charset);
        return FAILURE;
    }

    uint16_t width = GFX_TEX_gw(loaded_charset.characters);
    uint16_t height = GFX_TEX_gh(loaded_charset.characters);
    loaded_charset.char_height = char_height;
    loaded_charset.char_width = char_width;
    loaded_charset.height = height / char_height;
    loaded_charset.width = width / char_width;

    uint16_t x, y, i;
    i = 0;
    for(y = 0; y < height; y += char_height) {
        for(x = 0; x < width; x += char_width) {
            loaded_charset.rects[i].w = loaded_charset.char_width;
            loaded_charset.rects[i].h = loaded_charset.char_height;
            loaded_charset.rects[i].x = x;
            loaded_charset.rects[i].y = y;
            i++;
        }
    }

    hlp_log_ln(HLP_LOG_INFO, "TEXT Engine Init OK.");
    return SUCCESS;
}

void TEXT_blit_formatted(TEXT_Point* pos, const char* format, ...) {

    va_list args;
    va_start (args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    internal_blit(pos, buffer);
}

void TEXT_blit(TEXT_Point* pos, char* text) {

    internal_blit(pos, text);
}

void TEXT_quit() {

    hlp_log_ln(HLP_LOG_INFO, "Quitting the TEXT system.");
    GFX_free(loaded_charset.characters);
    hlp_log_ln(HLP_LOG_INFO, "TEXT system has quitted.");
}

TEXT_Rect TEXT_precal_rect(char* text) {

    uint16_t i;
    uint16_t line_jumps = 0;
    uint16_t x = 0;
    uint16_t longest_line = 0;

    for(i = 0; text[i] != '\0'; i++){
        char c = text[i];
        if (c == ' ') {
            x++;
            continue;
        }
        if(c == '\n') {
            line_jumps++;
            if(x > longest_line) {
                longest_line = x;
            }
            x = 0;
            continue;
        }
        else {
            x++;
        }
    }

    if (longest_line == 0) longest_line = x;
    if (x > longest_line) longest_line = x;

    uint16_t width = longest_line * loaded_charset.char_width;
    uint16_t height = (line_jumps + 1) *loaded_charset.char_height;
    TEXT_Rect rect = {  .w = width, .h = height};
    return rect;
}

//static funcs
static void internal_blit(TEXT_Point* pos, char* text) {

    uint16_t i;
    uint16_t line_jumps = 0;
    uint16_t x = 0;
    uint16_t longest_line = 0;

    for(i = 0; text[i] != '\0'; i++){
        char c = text[i];
        if (c == ' ') {
            x++;
            continue;
        }
        if(c == '\n') {
            line_jumps++;
            if(x > longest_line) {
                longest_line = x;
            }
            x = 0;
            continue;
        }
        else {
            x++;
        }
        GFX_Rect dest_rect = { .x = pos->x + (x - 1) * loaded_charset.char_width,
                               .y = pos->y + line_jumps*loaded_charset.char_height,
                               .w = loaded_charset.char_width,
                               .h = loaded_charset.char_height
                            };
        GFX_blit(loaded_charset.characters, &loaded_charset.rects[(int) c -1], &dest_rect);
    }
}
