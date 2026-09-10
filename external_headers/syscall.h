#ifndef NATIVE_SYSCALL_E_H
#define NATIVE_SYSCALL_E_H
enum nat_Syscall_Type {
    nat_threadCreate_e,
    nat_threadJoin_e,
    nat_logError_e,
};
typedef struct nat_Syscall_arg nat_Syscall_arg;
typedef int(nat_Syscall_arg_cleanup)(nat_Syscall_arg* arg);
struct nat_Syscall_arg {
    void* arg;
    nat_Syscall_arg_cleanup* cleanup;
};
typedef struct nat_Syscall_Request nat_Syscall_Request;
nat_Syscall_Request* nat_pushToSyscallQueue(void* funcPtr, nat_Syscall_arg* arg);
int nat_checkSyscallRequest(nat_Syscall_Request* request);
void nat_waitSyscallRequest(nat_Syscall_Request* request);
#endif
