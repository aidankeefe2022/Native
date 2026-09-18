#ifndef INTERNAL_NATIVE_BUFFER_H
#define INTERNAL_NATIVE_BUFFER_H
#include <internal_headers/common.h>

typedef struct nat_Buffer {
    u8* buffer;
    u64 len;
    u64 cap;
    u8  dynamic;
}nat_Buffer;

#define ForEach(indexVar, buffer)  for (u64 indexVar = 0; indexVar < buffer.len; indexVar++)

#endif /* INTERNAL_NATIVE_BUFFER_H */
