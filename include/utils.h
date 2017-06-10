#ifndef __UTILS_H
#define __UTILS_H

#include <stdio.h>

#define SUCCESS 0
#define ERROR -1

#if DEBUG
#define DEBUG_PRINT(format, ...) \
    fflush(stdout); \
    fprintf(stderr, "[DEBUG]: "); \
    fprintf(stderr, format, ##__VA_ARGS__); \
    fprintf(stderr, "\t > File: %s \n", __FILE__); \
    fprintf(stderr, "\t > Function: %s \n", __func__); \
    fprintf(stderr, "\t > Line: %d \n", __LINE__)
#else
#define DEBUG_PRINT(format, ...) ;
#endif

#if ERRORS_INFO || DEBUG
#define STDERR_INFO(format, ...) \
    fflush(stderr); \
    fprintf(stderr, "[ERROR]: "); \
    fprintf(stderr, format, ##__VA_ARGS__); \
    fprintf(stderr, "\t > File: %s \n", __FILE__); \
    fprintf(stderr, "\t > Function: %s \n", __func__); \
    fprintf(stderr, "\t > Line: %d \n", __LINE__)
#else
#define STDERR_INFO(format, ...)
#endif

#define RAISE_INIT_ERROR() { STDERR_INFO("Mem. Alloc. Failure"); return ERROR; }

#endif //__UTILS_H