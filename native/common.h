//
// Created by aidankeefe on 3/15/26.
//

#ifndef AIDAN_COMMON_H
#define AIDAN_COMMON_H

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

#endif //WOLFHTTPS_COMMON_H
