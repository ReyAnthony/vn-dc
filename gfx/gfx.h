#ifndef GFX_H_INCLUDED
#define GFX_H_INCLUDED

/*
    This can be casted to be used directly,
    but the resulting code will not be lib agnostic.
    ==========>
    Please use the GFX_TEX helpers functions to interact with the underlying type.
*/
#define GFX_TEX_PTR void*

typedef struct {
    int x; int y; int w; int h;
} GFX_Rect;

#define GFX_Rect_default (GFX_Rect) {.x = 0, .y = 0, .w = -1, .h = -1}
#define GFX_Rect_init_pos(xx, yy) (GFX_Rect) {.x = xx, .y = yy, .w = -1, .h = -1}
#define GFX_Rect_init_all(xx, yy, ww, hh) (GFX_Rect) {.x = xx, .y = yy, .w = ww, .h = hh}

int GFX_init();
void GFX_quit();

//r,g,b are ranging from 0 to 1
void GFX_set_clear_color(float r, float g, float b);

void GFX_start_draw();
void GFX_end_draw();

/*
    Blits the specified src_rect of texture tex, at dest_rect.
    if src_rect is NULL, the whole texture is used.
    if dest_rect w and h are set to -1, the size will be the loaded texture size.
*/
void GFX_blit(GFX_TEX_PTR tex, GFX_Rect* src_rect, GFX_Rect* dest_rect);
void GFX_blit_s(GFX_TEX_PTR tex, GFX_Rect* dest_rect); //when no need to use src_rect ===> TODO use a macro instead

//variant that can modify alpha value when blitting (0 == transparent / 1 == full opacity)
void GFX_blit_a(GFX_TEX_PTR tex, GFX_Rect* src_rect, GFX_Rect* dest_rect, float alpha);

/*
    Returns NULL if the texture flag is unsupported.
    Can return NULL on other cases, see implementation for each backend.
*/
GFX_TEX_PTR GFX_allocate(char* fn, int load_flags, int tex_flags);

void GFX_free(GFX_TEX_PTR tex);

//==== GFX_TEX helpers
int GFX_TEX_gh(GFX_TEX_PTR tex);
int GFX_TEX_gw(GFX_TEX_PTR tex);

#endif // GFX_H_INCLUDED
