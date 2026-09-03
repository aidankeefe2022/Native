#include <internal_headers/native.h>
#include <internal_headers/syscall.h>
#include <external_headers/protectedThread.h>


i32 nat_SafeThread_Create(nat_SafeThread* safeThread,
        nat_SafeThread_Callback cb, void* ctx)
{

}

i32 nat_SafeThread_Join(nat_SafeThread* safeThread, int* ret)
{
    nat_pushToSyscallQueue(nat_threadJoin_e, );
}
