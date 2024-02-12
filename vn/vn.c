#include "vn.h"
#include <string.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "../gfx/gfx.h"

#if DC
#include "../gfx/arch/gfx_pvr.h"
#endif // DC

#define YUGI_CHAR PPC(yugi.png)
#define CLASSROOM PPC(classroom.png)
#define NEXT_MSG_ARROW PPC(arrow.png)
#define PANEL PPC(panel.png)
#define TEST PPC(resize_test.png)

static int gp_init();
static void gp_update(float delta_time);
static void gp_handle_input(SDL_Event* event, float delta_time);
static void gp_draw(float delta_time);
static void gp_quit();
static int gp_system_id;

int VN_Init(){

    hlp_log_ln(HLP_LOG_INFO, "Starting to init VN.");
    gp_system_id =
            ENGINE_subsystem_bind((Engine_SubsystemBind) {
                .init = gp_init,
                .update = gp_update,
                .handle_input = gp_handle_input,
                .draw = gp_draw,
                .quit = gp_quit,
              });

    hlp_log_ln(HLP_LOG_INFO, "Init VN finished.");
    return 1;
}

int VN_Quit(){

    hlp_log_ln(HLP_LOG_INFO, "Quitting VN.");
    //ENGINE_subsystem_unbind(gp_system_id);
    hlp_log_ln(HLP_LOG_INFO, "Quitted VN.");
    return 1;
}

//===========
static float timer;
static GFX_Rect text_panel_rect;

typedef struct {
    char* msg;
    int len;
    Uint16 index; //a bit useless, should not be duped acrros each msg

} VN_Message;

//needs struct
static VN_Message messages[3];
static int message_cnt;
static int current_msg_index;

static GFX_TEX_PTR character_A;
static GFX_TEX_PTR classroom_BKG;
static GFX_TEX_PTR panel;
static GFX_TEX_PTR next_msg_arrow;

static GFX_Rect next_msg_arrow_rect;
static float next_msg_alpha;
static GFX_Rect character_centered;

static GFX_TEX_PTR test;
static GFX_Rect centered;
static GFX_Rect off_centered;
static GFX_Rect off_centered_2;

static char buffer[1024];

//Use an interpreter to read the next story step ?
//seek on the file as needed ?
static int gp_init() {

    hlp_log_ln(HLP_LOG_INFO, "VN init.");

    messages[0].msg = "Hey ! I solved the millenium puzzle !";
    messages[1].msg = "This works !";
    messages[2].msg = "Very well ! Test !";
    message_cnt = sizeof(messages) / sizeof(messages[0]);
    current_msg_index = 0;

    for (int i = 0; i < message_cnt; i++) {
        messages[i].index = 0;
        messages[i].len = strlen(messages[i].msg);
    }

    //mext_msg
    next_msg_arrow_rect = GFX_Rect_init_pos(WIDTH - 32, HEIGHT - 32);
    next_msg_arrow = GFX_allocate(NEXT_MSG_ARROW, 0, 0);
    next_msg_alpha = 0;

    if(next_msg_arrow == NULL) {

        hlp_log_ln(HLP_LOG_ERROR, "Could not load %s", NEXT_MSG_ARROW);
        return 0;
    }

    //panel
    text_panel_rect = GFX_Rect_init_all(WIDTH/2, HEIGHT - HEIGHT/7, WIDTH, 150);

    #if DC
    panel = GFX_allocate(PANEL, 0, 0);
    #else
    panel = GFX_allocate(PANEL, 0, 0);
    #endif

    if(panel == NULL) {

        hlp_log_ln(HLP_LOG_ERROR, "Could not load panel");
        return 0;
    }

    //Init character
    character_A = GFX_allocate(YUGI_CHAR, 0, 0);
    character_centered = GFX_Rect_init_pos(WIDTH/2, HEIGHT/2 + GFX_TEX_gh(character_A) /8);

    if (character_A == NULL) {

        hlp_log_ln(HLP_LOG_ERROR, "Could not load %s", YUGI_CHAR);
        return 0;
    }


    #if DC
    classroom_BKG = GFX_allocate__no_png_alpha(CLASSROOM, 0, GFX_FILTER_NEAREST);
    #else
    classroom_BKG = GFX_allocate(CLASSROOM, 0, 0);
    #endif

    if(classroom_BKG == NULL) {

        hlp_log_ln(HLP_LOG_ERROR, "Could not load %s", CLASSROOM);
        return 0;
    }

    //TODO create a debug scene, to check GL is working
    //testing
    test = GFX_allocate(TEST, 0, 0);
    centered = GFX_Rect_init_pos(WIDTH/2, HEIGHT/2);
    off_centered = GFX_Rect_init_all(WIDTH/2 + 128, HEIGHT/2, 114, 120);
    off_centered_2 = GFX_Rect_init_all(WIDTH/2 - 128, HEIGHT/2, 128, 128);


    return 1;
}

static void gp_handle_input(SDL_Event* event, float delta_time) {


#ifdef PC
    if(event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_RETURN) {
#endif // PC
#ifdef DC
    if(event->type == SDL_JOYBUTTONDOWN && event->jbutton.button == DC_BUTTON_A) {
#endif // DC
       VN_Message* msg = &(messages[current_msg_index]);
       if(msg->index == msg->len) {
            //go to next message and clear text buffer surf
            if (current_msg_index < 3 - 1) {
                 timer = 0;
                 current_msg_index++;
            }
       }
       else {
            //blit all the message if we pressed but was not finished yet.
            //I was using memccpy (with a 0), was it a typo ? it's not c99 compliant
            memcpy(buffer, msg->msg, msg->len * sizeof(char));
            buffer[msg->len] ='\0';
            msg->index = msg->len;
       }
    }
}

static float timer_sin = 0;
static void gp_update(float delta_time) {

    timer += delta_time;
    timer_sin += delta_time;

    VN_Message* msg = &(messages[current_msg_index]);
    if(msg->index == msg->len) {

       next_msg_alpha = (sin(timer_sin * 2) + 1) / 2;
    }
    else {

        if (msg->index == 0) {
            buffer[msg->index] = msg->msg[msg->index];
            buffer[msg->index  + 1] = '\0';
        }

        if (msg->index < msg->len && timer > 0.05f)
        {
            msg->index++;
            buffer[msg->index] = msg->msg[msg->index];
            buffer[msg->index  + 1] = '\0';

            timer = 0;
        }
    }
}

static void gp_draw(float delta_time) {

    GFX_blit_s(classroom_BKG, NULL);
    GFX_blit_s(character_A, &character_centered);

    GFX_blit_s(test, &centered);
    GFX_blit_s(test, &off_centered);
    GFX_blit_s(test, &off_centered_2);

    GFX_blit_a(panel, NULL, &text_panel_rect, 0.8f);

    VN_Message* msg = &(messages[current_msg_index]);
    TEXT_Point pt = TEXT_Point_init_pos(text_panel_rect.x - WIDTH/2 + 32 , HEIGHT - 120);
    TEXT_blit(&pt, buffer);

    if(msg->index == msg->len) {

        GFX_blit_a(next_msg_arrow, NULL, &next_msg_arrow_rect, next_msg_alpha);
    }
}

static void gp_quit() {

    hlp_log_ln(HLP_LOG_INFO, "Quitting VN gameplay, will cleanup.");

    GFX_free(character_A);
    GFX_free(classroom_BKG);
    GFX_free(panel);
    GFX_free(next_msg_arrow);

    //clear surfaces
   hlp_log_ln(HLP_LOG_INFO, "VN gameplay has quit");
}
