#include "gfx.h"

#include "../constants.h"
#include "../helpers.h"
#include "quick_lists.h"

#ifdef GLDC
    #include <GLDc/gl.h>
    #include <GLdc/glu.h>
    #include <GLdc/glkos.h>
#elif DC
    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#elif PC
    #include <windows.h>
    #include <gl/glew.h>
    #include <GL/glu.h>
    #include <SDL/SDL.h>
    #include "../path_helpers.h"
#endif // PC

#ifdef DC
    #include <png/png.h>
    #include <png/kospng.h>
    #include <stdlib.h>
    #include <stdio.h>
    //Debugging
    #include <kos/dbgio.h>
#else
    #include <stdint.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include "../_third_party/stb_image.h"

    //unused
    #define PNG_FULL_ALPHA 0
    #define PNG_NO_ALPHA 0

    //TODO use OPENGL types.
    typedef uint32_t uint32;
    typedef uint16_t uint16;
#endif

typedef struct {

    GLuint tex_id;
    uint32 w;
    uint32 h;
} tex_wrapper;

#define TEX_PTR tex_wrapper*

//only used to keep track of textures, aka used when freeing, otherwize the ptr is passed around.
static TList_wrapper textures;

static void internal_blit(GFX_TEX_PTR tex, GFX_Rect* src_rect, GFX_Rect* dest_rect, float alpha);
static GFX_TEX_PTR internal_allocate(char* fn, int load_flags, int tex_flags, int has_alpha);
static void load_png(const char *fn, GLuint *txr, uint32 *w, uint32 *h, int flags);

#if PC
//used to print an overlay for 16/9 gameplay
static GFX_TEX_PTR bkg_16_9;
static GFX_Rect bkg_16_9_rect;
#endif // PC

int GFX_init() {

    #if DC
    glKosInit();
    #else
    {
        hlp_log_ln(HLP_LOG_INFO, "Initialising GFX Engine");
        hlp_log_ln(HLP_LOG_INFO, "Starting up SDL 1.2");

        if(SDL_Init( SDL_INIT_VIDEO) < 0 ) {

            hlp_log_ln(HLP_LOG_ERROR, "Video initialization failed: %s", SDL_GetError());
            return -1;
        }

        SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
        SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
        SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

        hlp_log_ln(HLP_LOG_INFO, "Setting up video mode and requesting an OpenGL context");

        //On windows, we can and should request higher resolution.
        if( SDL_SetVideoMode(1280, 720, 16, SDL_OPENGL) == 0 ) {

            hlp_log_ln(HLP_LOG_ERROR,  "Video mode set failed: %s", SDL_GetError());
            return -1;
        }

        hlp_log_ln(HLP_LOG_INFO, "GLEW Init.");
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
             hlp_log_ln(HLP_LOG_ERROR, "Error: %s", glewGetErrorString(err));
            return -1;
        }

        if (!GLEW_VERSION_1_1)
        {
            hlp_log_ln(HLP_LOG_ERROR, "Dude, this computer is meant for spreadsheets.");
            return -1;
        }

        hlp_log_ln(HLP_LOG_INFO, "All clear ! Let's draw !");

        hlp_log_ln(HLP_LOG_INFO, "Renderer : %s", glGetString(GL_RENDERER));
        hlp_log_ln(HLP_LOG_INFO, "Renderer : %s", glGetString(GL_VERSION));
    }

    hlp_log_ln(HLP_LOG_INFO, "Allocating the overlay for Windows");

    //not a ^2 but we don't care on windows
    bkg_16_9 = GFX_allocate(PPC(16_9_overlay.png), 0, 0);
    bkg_16_9_rect = GFX_Rect_init_pos(320, 240);

    #endif

    GFX_set_clear_color(0, 0, 0);
    glClearDepth(1.0f);

    glShadeModel(GL_FLAT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    {
        //bottom is 480, so that we invert the coordinate system
        #if PC
            glOrtho(-112, 854 - 112, 480, 0, -1, 1);
        #else
        glOrtho(0, 640, 480, 0, -1, 1);
        #endif
        //Great streamlined GL tut : http://www.sdltutorials.com/sdl-opengl-tutorial-basics
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_TEXTURE_2D);


        //DO NOT USE DECAL, it will break transparency
        //why did I needed it for anyway ?
        //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    }
    glLoadIdentity();

    //init the pool of textures
    textures = ql_create_list();

    return 0;
}

void GFX_quit() {

    hlp_log_ln(HLP_LOG_INFO, "GFX engine is quitting, cleanup...");


    if(textures.count > 0) {
        hlp_log_ln(HLP_LOG_INFO, "GFX engine will clean stuff that systems might not have unallocated by themselves.");
        hlp_log_ln(HLP_LOG_WARN, "This might indicate that something has not cleared its junk...");
    }
    else {

        hlp_log_ln(HLP_LOG_INFO, "GFX engine has no texture to free, seems like systems have cleaned their junk !");
    }

    while(textures.count > 0) {

       GFX_free(textures.elements[textures.count -1]);
    }
    ql_delete_list(&textures);

    #if PC
    SDL_Quit();
    #endif
    //glew quit ?

    hlp_log_ln(HLP_LOG_INFO, "GFX engine has quit.");

}

void GFX_set_clear_color(float r, float g, float b) {

    glClearColor(r, g, b, 0);
}

void GFX_start_draw() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void GFX_end_draw() {

    #ifdef GLDC
    glKosSwapBuffers();
    #elif defined(DC)
    glutSwapBuffers();
    #else

    GFX_blit_s(bkg_16_9, &bkg_16_9_rect);
    SDL_GL_SwapBuffers();
    SDL_Delay(10);
    #endif

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

//only ^2 , try with GL_RECTANGLE_2D for NPOT textures ?
GFX_TEX_PTR GFX_allocate(char* fn, int load_flags, int tex_flags) {

    return internal_allocate(fn, load_flags, tex_flags, 1);
}

void GFX_free(GFX_TEX_PTR tex) {

    hlp_log(HLP_LOG_INFO, "TXR unloading %d \n", ((TEX_PTR)tex)->tex_id);

    glDeleteTextures(1, &((TEX_PTR)tex)->tex_id);
    ql_remove(&textures, tex); //free it from the list
    free(tex); //cleanup the allocated stuff

    //checking the remaining
    hlp_log_ln(HLP_LOG_TRACE, "Still in memory : (count %d) : ", textures.count);

    for(int i = 0; i < textures.count; i++) {

       GLuint t = ((TEX_PTR) textures.elements[i])->tex_id;
       hlp_log_ln(HLP_LOG_TRACE, "%hu, ", t);
    }

}

int GFX_TEX_gh(GFX_TEX_PTR tex) {

    return ((TEX_PTR)tex)->h;
}

int GFX_TEX_gw(GFX_TEX_PTR tex) {

  return ((TEX_PTR)tex)->w;
}

//This should not exist on Windows, it's a convenience extension for Dreamcast
//This force to ignore the alpha and thus loads the image with better quality (see load_png)
GFX_TEX_PTR GFX_allocate__no_png_alpha(char* fn, int load_flags, int tex_flags) {

    return internal_allocate(fn, load_flags, tex_flags, 0);
}

static GFX_TEX_PTR internal_allocate(char* fn, int load_flags, int tex_flags, int has_alpha) {

    if(load_flags == -1 || tex_flags == -1) {
        return NULL;
    }

    uint32 w = -1;
    uint32 h = -1;
    GLuint id = -1;

    load_png(fn, &id, &w, &h, has_alpha == 1 ? PNG_FULL_ALPHA : PNG_NO_ALPHA);
    if(id == -1) {

        hlp_log_ln(HLP_LOG_ERROR, "TXR Could not load %s, aborting.", fn);
        return NULL;
    }

    hlp_log_ln(HLP_LOG_INFO, "TXR loading %d, %s", id, fn);
    tex_wrapper* tex = (tex_wrapper*) malloc(sizeof(tex_wrapper));
    tex->tex_id = id;
    tex->w = w;
    tex->h = h;

    ql_append(&textures, tex);

    return tex;
}

static void internal_blit(GFX_TEX_PTR tex, GFX_Rect* src_rect, GFX_Rect* dest_rect, float alpha) {

    GFX_Rect r;

    if (dest_rect == NULL) {

        r = GFX_Rect_init_pos(GFX_TEX_gw(tex)/2, GFX_TEX_gh(tex)/2);
        dest_rect = &r;
    }

    float w = dest_rect->w == -1 ? GFX_TEX_gw(tex) : dest_rect->w;
    float h = dest_rect->h == -1 ? GFX_TEX_gh(tex) : dest_rect->h;

    float min_x = ((float) dest_rect->x) - ((float) w) /2.0f;
    float max_x = ((float) dest_rect->x) + ((float) w) /2.0f;
    float min_y = ((float) dest_rect->y) - ((float) h) /2.0f;
    float max_y = ((float) dest_rect->y) + ((float) h) /2.0f;

    glColor4f( 1.0f, 1.0f, 1.0f, alpha);

    if (src_rect == NULL) {

        //draw the quad at the position and set the texture to fill the whole quad
        glBindTexture(GL_TEXTURE_2D, ((TEX_PTR)tex)->tex_id);
        glColor4f(1.0, 1.0, 1.0, alpha); // reset gl color
        glBegin(GL_QUADS);
        {
            glTexCoord2f( 0, 0 ); glVertex3f(min_x, min_y, 0.0f);
            glTexCoord2f( 0, 1 ); glVertex3f(min_x, max_y, 0.0f);
            glTexCoord2f( 1, 1 ); glVertex3f(max_x, max_y, 0.0f);
            glTexCoord2f( 1, 0 ); glVertex3f(max_x, min_y, 0.0f);
        }
        glEnd();

    }
    else {

        float tex_min_x =  ((float) src_rect->x)/GFX_TEX_gw(tex);
        float tex_min_y =  ((float) src_rect->y)/GFX_TEX_gh(tex);
        float tex_max_x =  ((float) src_rect->x + (float) src_rect->w)/GFX_TEX_gw(tex);
        float tex_max_y =  ((float) src_rect->y + (float) src_rect->h)/GFX_TEX_gh(tex);

        glBindTexture(GL_TEXTURE_2D, ((TEX_PTR)tex)->tex_id);
        glColor4f(1.0, 1.0, 1.0, alpha); // reset gl color
        glBegin(GL_QUADS);
        {
            glTexCoord2f( tex_min_x, tex_min_y ); glVertex3f(min_x, min_y, 0.0f);
            glTexCoord2f( tex_min_x, tex_max_y ); glVertex3f(min_x, max_y, 0.0f);
            glTexCoord2f( tex_max_x, tex_max_y ); glVertex3f(max_x, max_y, 0.0f);
            glTexCoord2f( tex_max_x, tex_min_y ); glVertex3f(max_x, min_y, 0.0f);
        }
        glEnd();
    }
}

//https://dcemulation.org/phpBB/viewtopic.php?t=103270
static void load_png(const char *fn, GLuint *txr, uint32 *w, uint32 *h, int flags) {

#if DC
    uint32 iw , ih = 0;
    kos_img_t tex;

    if(png_to_img(fn, flags, &tex)) {

        *txr = -1;
        hlp_log_ln(HLP_LOG_ERROR, "Could not load %s, is the file missing ?", fn);
        return;
    }

    *w = iw = tex.w;
    *h = ih = tex.h;
    glGenTextures(1, txr);

    //format should be the same as internal format (GL_RGBA) AND NOT PVR_TXRFMT_ARGB4444
    glBindTexture(GL_TEXTURE_2D, *txr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //disable mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (KOS_IMG_FMT_I(tex.fmt) == KOS_IMG_FMT_ARGB4444) { //loaded as transparent with PNG_FULL_ALPHA flag

        #if GLDC
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iw, ih, 0, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV , tex.data);
        #else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iw, ih, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4 , tex.data);
        #endif
    }
    else if (KOS_IMG_FMT_I(tex.fmt) == KOS_IMG_FMT_RGB565) {

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iw, ih, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, tex.data);
    }
    else {

        hlp_log_ln(HLP_LOG_ERROR, "TXR failed to load %s. format is %lu", fn, KOS_IMG_FMT_I(tex.fmt));
        exit(1);
    }

    kos_img_free(&tex, 0);
#else
    //We ignore flags here, no need to load without alpha or anything since we'll always draw with max quality
    unsigned char* tex = stbi_load(fn, (int*) w, (int*) h, 0, STBI_rgb_alpha);
    if(tex == NULL) {

        hlp_log_ln(HLP_LOG_ERROR, "Could not load %s, is the file missing ?", fn);
        return;
    }

    {
        glGenTextures(1, txr);
        glBindTexture(GL_TEXTURE_2D, *txr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //disable mipmaps
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *w, *h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
    }

    stbi_image_free(tex);
#endif
}
