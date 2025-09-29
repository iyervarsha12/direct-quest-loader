// Helper library functions for the direct quest loader
#ifndef _QUESTBINLOADER_HELPER_H_
#define _QUESTBINLOADER_HELPER_H_

#include <unistd.h>
#include <libelf.h>

//#define DEBUG 1
#define MAX_ARGS_LENGTH 80

#ifdef DEBUG
    #define DEBUG_PRINT(fmt, ...) \
        fprintf(stderr, "DEBUG: " fmt "\n", ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...) // nothing
#endif

int send_to_guest(int coreid, void* data, size_t sz);

#endif
