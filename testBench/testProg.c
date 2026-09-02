#include "../native/native.h"
#include <stdio.h>
#include <threads.h>

int threadRun(void* ctx) {
    const char* string = ctx;
    printf("%s", string);
    return 0;
}

int run(void) {
    printf("This is a test program");
    thrd_t thread = {0};
    thrd_create(&thread, threadRun, "this is my ctx");
    thrd_join(thread, NULL);
    return 0;
}
