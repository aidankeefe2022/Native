#ifndef INTERNAL_NATIVE_BUFFER_H
#define INTERNAL_NATIVE_BUFFER_H
#include <internal_headers/common.h>

typedef struct Native_Buffer {
    u64 cap;
    u64 len;
    u8* buf;
}Native_Buffer;
#endif /* INTERNAL_NATIVE_BUFFER_H */
