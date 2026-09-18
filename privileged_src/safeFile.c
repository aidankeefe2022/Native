#include <internal_headers/common.h>
#include <internal_headers/safeFile.h>
#include <stdlib.h>
#include <stdio.h>


i32 nat_writeFile(struct nat_writeFileArg* arg) {
    nat_Buffer fileName = arg->fileName;
    nat_Buffer data = arg->data;
    u8* sanFileName = malloc(fileName.len);
    Defer(free(sanFileName)) {
        u8* tracker = sanFileName;
        ForEach(i, fileName) {
            if (fileName.buffer[i] == '/') {
                continue;
            }
            else {
                *tracker = fileName.buffer[i];
                tracker++;
            }
        }
        FILE* fp = fopen((char*)sanFileName, "w");
        if (fp == NULL)
            DeferBreak;
        fwrite(data.buffer, 1, data.len, fp);
    }
    return 0;
}
