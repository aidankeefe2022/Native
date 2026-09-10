#include <internal_headers/common.h>
#include <external_headers/logging.h>
#include <internal_headers/logging.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

i32 nat_logError(struct nat_logErrorArg* arg)
{
    /* The caller blocks in nat_logErrorI until this returns, and calls va_end
       itself -- it owns the frame the va_list points into. */
    int ret = vfprintf(stderr, arg->fmt, arg->list);
    va_end(arg->list);
    return ret;
}
