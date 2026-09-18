#ifndef NATIVE_SAFEFILE_I_H
#define NATIVE_SAFEFILE_I_H
#include <internal_headers/common.h>

struct nat_writeFileArg {
    nat_Buffer fileName;
    nat_Buffer data;
};
i32 nat_writeFile(struct nat_writeFileArg* arg);
i32 nat_readFile(struct nat_writeFileArg* arg);

#endif
