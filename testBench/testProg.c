#include <stdio.h>
#include <threads.h>
#include <external_headers/syscall.h>

int threadRun(void* ctx) {
    const char* string = ctx;
    printf("%s", string);
    return 0;
}

int run(void) {
    printf("This is a test program");
    nat_pushToSyscallQueue(nat_logError_e, "This is my error");
    return 0;
}
