#ifndef HELPERS_H_INCLUDED
#define HELPERS_H_INCLUDED

#include <time.h>
#include <string.h>

#if NO_LOGGING
    #define HLP_LOG_INFO
    #define HLP_LOG_WARN
    #define HLP_LOG_ERROR
    #define HLP_LOG_TRACE

    #define HLP_LOG_LEVEL_ALL

    #define __FILENAME__

    #define hlp_log(trace_level, format_string, ...)
    #define hlp_log_ln(trace_level, format_string, ...)

#else

    #define __FILENAME__ strstr(__FILE__, "morstairs")

    #define HLP_LOG_INFO_STR  "[INFO]  "
    #define HLP_LOG_WARN_STR  "[WARN]  "
    #define HLP_LOG_ERROR_STR "[ERROR] "
    #define HLP_LOG_TRACE_STR "[TRACE] "

    //log levels
    #define HLP_LOG_LEVEL_ALL  (HLP_LOG_INFO|HLP_LOG_ERROR|HLP_LOG_WARN)

    #define HLP_LOG_INFO  0x0001
    #define HLP_LOG_ERROR 0x0010
    #define HLP_LOG_WARN  0x0100
    #define HLP_LOG_TRACE 0x1000

    //custom value (should be somewhere else or defined in conf but who cares)
    #define HLP_LOG_LEVEL (HLP_LOG_ERROR|HLP_LOG_WARN|HLP_LOG_TRACE|HLP_LOG_INFO)

    #ifndef HLP_LOG_LEVEL //can be set by user at compile time
    #define HLP_LOG_LEVEL  HLP_LOG_LEVEL_ALL
    #endif

    #define is_level_ok(level_to_test) ((HLP_LOG_LEVEL & level_to_test) == level_to_test)

    //only support defined constant not OR'd ones (obviously)
    #define bits_to_hr(bits)\
        (bits == HLP_LOG_INFO ? HLP_LOG_INFO_STR \
         : bits == HLP_LOG_ERROR ? HLP_LOG_ERROR_STR \
         : bits == HLP_LOG_WARN ? HLP_LOG_WARN_STR \
         : bits == HLP_LOG_TRACE ? HLP_LOG_TRACE_STR : "")

    #define hlp_log(trace_bits, format_string, ...)\
        {\
        char * b = bits_to_hr(trace_bits);\
        if (is_level_ok(trace_bits)) \
            fprintf(stdout, "%ld %s [%s] in %s() @ L %d // " format_string, \
                    time(NULL), b,__FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ );\
        } (void)0 //hack to require a ;

    #define hlp_log_ln(trace_level, format_string, ...) hlp_log(trace_level, format_string "\n", ##__VA_ARGS__)

#endif // NO_LOGGING
#endif // HELPERS_H_INCLUDED
