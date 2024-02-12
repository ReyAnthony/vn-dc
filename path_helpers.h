#ifndef PATH_HELPERS_H_INCLUDED
#define PATH_HELPERS_H_INCLUDED

#ifdef DC
    #define RD(path) "rd/"#path
    #define PPC(path) "rd/"#path
#else
    #define RD(path) "romdisk/"#path
    #define PPC(path) "romdisk/"#path
#endif

#endif // PATH_HELPERS_H_INCLUDED
