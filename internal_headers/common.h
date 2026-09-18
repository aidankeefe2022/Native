//
// Created by aidankeefe on 3/15/26.
//

#ifndef NATIVE_COMMON_I_H
#define NATIVE_COMMON_I_H

#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef float f32;
typedef double f64;

#define KiB(x) ((uint64_t)(x) << 10)
#define MiB(x) ((uint64_t)(x) << 20)
#define GiB(x) ((uint64_t)(x) << 30)
#define TiB(x) ((uint64_t)(x) << 40)


/* ------ Error Codes ----- */

enum ErrorCodes {
    nat_MemError = -1,
    nat_BadFuncError = -2,
    /* Syscall could not complete yet; the caller should retry.  Used by
       nat_threadJoin so the dispatch loop never parks in a blocking join. */
    nat_WouldBlock = -3,
};


/* ----- MACRO CONSTRUCTIONS ----- */

#define Defer(statement) for (i32 _defer_name = 0; _defer_name < 1; _defer_name++, statement)
#define DeferBreak continue


#endif //NATIVE_COMMON_I_H
