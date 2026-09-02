#include <threads.h>
#include <native/common.h>


typedef struct SafeThread {
    thrd_t thread;
}SafeThread;


i32 native_threadCreate(SafeThread* thread) {
    
}
