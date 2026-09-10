#include <external_headers/protectedThread.h>
#include <external_headers/logging.h>
#include <stdio.h>
#include <unistd.h>

int threadRun(void* ctx) {
    (void)ctx;
    nat_logErrorI("This is my message!\n");
    nat_logErrorI("I am doing a ton of work!");
    return 0;
}

int run(void) {
    thrd_t thread = {0};
    int ret;
    thrd_create(&thread, threadRun, (void*)0);
    ret = fork();
    nat_logErrorI("fork return code: %d", ret);
    thrd_join(thread, &ret);
    return 0;
}
