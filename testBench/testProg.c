#include <external_headers/protectedThread.h>
#include <external_headers/logging.h>

int threadRun(void* ctx) {
    (void)ctx;
    nat_logErrorI("This is my message!\n");
    return 0;
}

int run(void) {
    /*
    nat_SafeThread* thread = (void*)0;
    int* ret = 0;
    nat_SafeThread_CreateI(&thread, threadRun, (void*)0);
    nat_SafeThread_JoinI(thread, ret);
    */
    nat_logErrorI("This is my message!\n");
    return 0;
}
