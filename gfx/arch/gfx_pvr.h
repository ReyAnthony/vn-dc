#ifndef GFX_PVR_H_INCLUDED
#define GFX_PVR_H_INCLUDED

//Store stuff specific to the arch, like texture flags..
#define GFX_FILTER_NEAREST      0
#define GFX_FILTER_BILINEAR     2
#define GFX_FILTER_TRILINEAR1   4
#define GFX_FILTER_TRILINEAR2   6

//this is good
/*
 PVR_TXRLOAD_FMT_NOTWIDDLE
 PVR_TXRLOAD_FMT_TWIDDLED
PVR_TXRLOAD_INVERT_Y */

//Those are wrong
#define GFX_TEX_4BPP    0x01
#define GFX_TEX_8BPP    0x02
#define GFX_TEX_16BPP   0x03

//Specific impl for PVR backend
/*
    use_alpha ? PVR_TXRFMT_ARGB4444 : PVR_TXRFMT_RGB565
    PVR_TXRFMT_ARGB4444 are lower quality.

*/
GFX_TEX_PTR GFX_allocate__no_png_alpha(char* fn, int load_flags, int tex_flags);

#endif // GFX_PVR_H_INCLUDED
