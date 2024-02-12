#include "audio.h"

#include "../helpers.h"
#define FAILURE 0
#define SUCCESS 1

#ifdef PC
   static Mix_Music* music;
#endif

static char current_music[255];

int AUDIO_init() {
     hlp_log_ln(HLP_LOG_INFO, "Audio engine is initializing");
#ifdef DC
    snd_stream_init();
    sndoggvorbis_init();
    return SUCCESS;
#endif
#ifdef PC
   if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1) {

        hlp_log_ln(HLP_LOG_ERROR, "Audio engine could not initialize");
        return FAILURE;
   }
#endif
    hlp_log_ln(HLP_LOG_INFO, "Audio engine has initialized");
    return SUCCESS;
}

void AUDIO_play(const char* file, int loop) {

//don't play the same music again
if(strcmp(current_music, file) == 0) {
    return;
}

#ifdef DC
    sndoggvorbis_stop();
    sndoggvorbis_start(file, loop);
#endif
#ifdef PC
    if(music != NULL) {
        Mix_FreeMusic(music);
    }
    if(loop == 1) {
        loop = -1;
    }
    music = Mix_LoadMUS(file);
    Mix_PlayMusic(music, loop);
#endif

strcpy(current_music, file);
}

void AUDIO_quit() {

    hlp_log_ln(HLP_LOG_INFO, "Audio engine is quitting.");
    #ifdef DC
    sndoggvorbis_stop();
    sndoggvorbis_shutdown();
    snd_stream_shutdown();
    #endif
    #ifdef PC
    Mix_CloseAudio();
    #endif
    hlp_log_ln(HLP_LOG_INFO, "Audio engine has quit.");
}



