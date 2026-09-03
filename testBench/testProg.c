#include <../external_headers/protectedThread.h>
#include <stdio.h>
#include <threads.h>

int threadRun(void* ctx) {
    const char* string = ctx;
    printf("%s", string);
    return 0;
}

int run(void) {
    printf("This is a test program");
    nat_SafeThread* thread = NULL;
    native_threadCreate(thread, threadRun, "this is my ctx");
    native_threadJoin(thread, NULL);
    return 0;
}
