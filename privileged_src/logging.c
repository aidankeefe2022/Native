#include <internal_headers/common.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

i32 nat_logError(FILE* stream, const char* fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    i32 ret = vfprintf(stream, fmt, list);
    va_end(list);
    return ret;
}
