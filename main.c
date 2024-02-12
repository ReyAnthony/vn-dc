#ifdef DC
#include <kos.h>
#include <arch/arch.h>

#define PROJECT_NAME "VN_DC"

#if defined(DEBUG) && defined(DC)
#include <arch/gdb.h>
#endif
KOS_INIT_FLAGS(INIT_DEFAULT | INIT_MALLOCSTATS);
extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);
#endif

//common includes
#include "engine.h"
#include "vn/game.h"

void exit_cleanup();
void exit_cleanup() {

    hlp_log_ln(HLP_LOG_INFO, "Reached the atexit cleanup callback.");
    GAME_quit();
    ENGINE_quit();
#ifdef DC
     arch_reboot(); //if we reboot we don't get the stats
#endif // DC
}

//TODO log more !
//TODO kos licence infos
//http://gamedev.allusion.net/docs/kos-2.0.0/arch_8h.html
//kos_get_license();
//kos_get_authors();

int main(int argc, char **argv) {
#if defined(DEBUG) && defined(DC)
	gdb_init();
	printf("Connection established to %s!", PROJECT_NAME);
#endif

    hlp_log_ln(HLP_LOG_INFO, "Program started.");

    if(!ENGINE_init()) {

        hlp_log_ln(HLP_LOG_ERROR, "Failed to init ENGINE, aborting.");
        return 1;
    }

    if(!GAME_init()) {

        hlp_log_ln(HLP_LOG_ERROR, "Could not init GAME engine.");
        return 1;
    }

    atexit(exit_cleanup);
    ENGINE_start();
    return 0;
}
