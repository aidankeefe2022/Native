#include <native/common.h>

typedef struct Native_Buffer {
    u64 cap;
    u64 len;
    u8* buf;
}Native_Buffer;
