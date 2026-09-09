#ifndef NATIVE_LOGGER_I_H
#define NATIVE_LOGGER_I_H
#include <stdarg.h>
#include <internal_headers/common.h>

struct nat_logErrorArg {
    const char* fmt;
    va_list list;
};

typedef i32(nat_logError_t)(struct nat_logErrorArg* arg);
nat_logError_t nat_logError;
#endif
