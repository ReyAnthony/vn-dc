#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED

#ifdef PC
#define WIDTH 640
#define HEIGHT 480
#define FONT_H 16
#define FONT_W 16
#endif

#ifdef DC
#define WIDTH 640
#define HEIGHT 480
#define FONT_H 16
#define FONT_W 16

#ifdef GL_DC
    #define DC_BUTTON_A SDL_DC_A
    #define DC_BUTTON_B SDL_DC_B
    #define DC_BUTTON_X SDL_DC_X
    #define DC_BUTTON_Y SDL_DC_Y
    #define DC_BUTTON_START SDL_DC_START
#else
    #define DC_BUTTON_A 0
    #define DC_BUTTON_B 1
    #define DC_BUTTON_X 2
    #define DC_BUTTON_Y 3
    #define DC_BUTTON_START 4
#endif

#define DC_STICK_Y_AXIS 0
#define DC_STICK_X_AXIS 1
#define DC_SHOULDER_L 3
#define DC_SHOULDER_R 2

#define DC_HAT_CROSS 0
#define DC_HAT_CROSS_UP 1
#define DC_HAT_CROSS_DOWN 4
#define DC_HAT_CROSS_LEFT 8
#define DC_HAT_CROSS_RIGHT 2
#endif

#endif // CONSTANTS_H_INCLUDED
