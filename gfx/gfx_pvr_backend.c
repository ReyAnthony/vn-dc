#include "gfx.h"
#include "arch/gfx_pvr.h"

#include <plx/texture.h>
#include <plx/sprite.h>
#include <plx/context.h>
#include <plx/dr.h>
#include <plx/list.h>
#include <plx/color.h>
#include <plx/matrix.h>
#include <png/png.h>
#include <string.h>

#define TEX_PTR plx_texture_t*

static plx_dr_state_t dr_state;
static void internal_blit(GFX_TEX_PTR tex, GFX_Rect* src_rect, GFX_Rect* dest_rect, float alpha);

/*

To load something from SH4 mem into another texture :
void pvr_txr_load_ex(void * src, pvr_ptr_t dst, uint32 w, uint32 h, uint32 flags);
void pvr_txr_load(void * src, pvr_ptr_t dst, uint32 count);

png_to_texture could also be useful ?

We seem to be able to do render to texture !!
see kallistios\examples\dreamcast\pvr\texture_render\texture_render.c
*/
int GFX_init() {

    vid_set_mode(DM_640x480_VGA, PM_RGB565);
    pvr_init_defaults();
    pvr_set_bg_color(1.0f, 1.0f, 1.0f);
    plx_dr_init(&dr_state);

    return 0;
}

void GFX_quit() {

    //Nothing to do for this backend
}

void GFX_set_clear_color(float r, float g, float b)
{
    pvr_set_bg_color(r, g, b);
}

void GFX_start_draw() {

    plx_scene_begin();
    plx_list_begin(PLX_LIST_TR_POLY);
}

void GFX_end_draw() {

    plx_scene_end();
}
void GFX_blit_s(GFX_TEX_PTR tex, GFX_Rect* dest_rect) {

    internal_blit(tex, NULL, dest_rect, 1);
}

void GFX_blit_a(GFX_TEX_PTR tex, GFX_Rect* src_rect, GFX_Rect* dest_rect, float alpha) {

    internal_blit(tex, src_rect, dest_rect, alpha);
}

void GFX_blit(GFX_TEX_PTR tex, GFX_Rect* src_rect, GFX_Rect* dest_rect) {

    internal_blit(tex, src_rect, dest_rect, 1);
}

GFX_TEX_PTR GFX_allocate(char* fn, int load_flags, int tex_flags) {

    if(load_flags == -1 || tex_flags == -1) {
        return NULL;
    }

    /*
        This backend only supports PNG.
        TODO check file is PNG or return NULL;
        Also, this backend can only load ^2 textures,
        we should check, otherwise the console CRASHES.

        only allow ^2 textures
        https://dcemulation.org/index.php?title=Texture_Formats
        maybe using strided textures could help ?

        use_alpha ? PVR_TXRFMT_ARGB4444 : PVR_TXRFMT_RGB565
    */
    TEX_PTR txr = plx_txr_load(fn, PNG_FULL_ALPHA, load_flags);
    plx_txr_setfilter(txr, tex_flags);

    return txr;
}

//todo do internal allocate to avoid duplication
GFX_TEX_PTR GFX_allocate__no_png_alpha(char* fn, int load_flags, int tex_flags) {

     if(load_flags == -1 || tex_flags == -1) {
        return NULL;
    }

    TEX_PTR txr = plx_txr_load(fn, PNG_NO_ALPHA, load_flags);
    plx_txr_setfilter(txr, tex_flags);

    return txr;
}

void GFX_free(GFX_TEX_PTR tex) {

    plx_txr_destroy(tex);
}

int GFX_TEX_gh(GFX_TEX_PTR tex) {

    TEX_PTR t = (TEX_PTR) tex;
    return t->h;
}

int GFX_TEX_gw(GFX_TEX_PTR tex) {

    TEX_PTR t = (TEX_PTR) tex;
    return t->w;
}

static void internal_blit(GFX_TEX_PTR tex, GFX_Rect* src_rect, GFX_Rect* dest_rect, float alpha) {

    TEX_PTR t = (TEX_PTR) tex;
    GFX_Rect r;

    if (dest_rect == NULL) {

        r = GFX_Rect_init_pos(t->w/ 2, t->h/2);
        dest_rect = &r;
    }

    float w = dest_rect->w == -1 ? t->w : dest_rect->w;
    float h = dest_rect->h == -1 ? t->h : dest_rect->h;

    if (src_rect == NULL) {

        plx_txr_send_hdr(t, PLX_LIST_TR_POLY, 0);
        plx_spr_fnd(&dr_state, w, h, dest_rect->x, dest_rect->y, 1.0f, alpha, 1.0f, 1.0f, 1.0f);
    }
    else {

        //it's actually plx_spr_fnd but with specifiable UVs
        //final size
        float w_size = w / 2.0f;
        float h_size = h / 2.0f;
        float x_dest = dest_rect->x;
        float y_dest = dest_rect->y;

        //UV and src_rect
        float src_x =  ((float) src_rect->x)/t->w;
        float src_y =  ((float) src_rect->y)/t->h;
        float src_w =  ((float) src_rect->x + (float) src_rect->w)/t->w;
        float src_h =  ((float) src_rect->y + (float) src_rect->h)/t->h;

        plx_txr_send_hdr(t, PLX_LIST_TR_POLY, 0);
        plx_vert_ffd(&dr_state, PLX_VERT,
                     x_dest - w_size,
                     y_dest + h_size,
                     1, alpha, 1, 1, 1,
                     src_x, src_h);

        plx_vert_ffd(&dr_state, PLX_VERT,
                     x_dest - w_size,
                     y_dest - h_size,
                     1, alpha, 1, 1, 1,
                     src_x, src_y);

        plx_vert_ffd(&dr_state, PLX_VERT,
                     x_dest + w_size,
                     y_dest + h_size,
                     1, alpha, 1, 1, 1,
                     src_w, src_h);

        plx_vert_ffd(&dr_state, PLX_VERT_EOS,
                     x_dest + w_size,
                     y_dest - h_size,
                     1, alpha, 1, 1, 1,
                     src_w, src_y);
    }
}
