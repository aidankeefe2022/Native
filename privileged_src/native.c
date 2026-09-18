#define _GNU_SOURCE  // memfd_create

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>  // memfd_create
#include <dlfcn.h>     // dynamic loading APIs
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <seccomp.h>

#include <external_headers/native.h>
#include <internal_headers/common.h>
#include <external_headers/protectedThread.h>
#include <internal_headers/syscall.h>

struct SpawnSafeThreadWithEntry_CTX {
    ProgramEntry entry;
    nat_SafeThread* self;
};

static i32 SpawnSafeThreadWithEntry(void* ctx) {
    struct SpawnSafeThreadWithEntry_CTX* stCtx = ctx;

    // runForignProgram has already returned by the time this runs, so the
    // context is heap allocated: unpack it and hand the memory back here.
    ProgramEntry entry = stCtx->entry;
    nat_SafeThread* self = stCtx->self;
    free(stCtx);

    i32 ret = entry();
    self->isRunning = 0;
    return ret;

}

struct ForignProgram_CTX {
    void* prog_handle;
    nat_SafeThread* main_ForignThread;
    u8 isError;
};

static struct ForignProgram_CTX runForignProgram(int fd){

    // Generate a virtual proc path referencing this file descriptor.
    char path[64];
    snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);

    // 4. Open the dynamic library (.so on Linux, .dylib on macOS).
    void *handle = dlopen(path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error opening library: %s\n", dlerror());
        close(fd);
        return (struct ForignProgram_CTX){.isError = 1};
    }

    // Clear any existing error.
    dlerror();
    close(fd);

    // Locate the function symbol inside the library.
    ProgramEntry forignRun = (ProgramEntry)dlsym(handle, "run");
    char *error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "Error locating symbol: %s\n", error);
        dlclose(handle);
        return (struct ForignProgram_CTX){.isError = 1};
    }

    // Use the function pointer normally.  The new thread outlives this frame,
    // so its context is heap allocated and freed by the thread itself.
    struct SpawnSafeThreadWithEntry_CTX* stCtx = malloc(sizeof(*stCtx));
    if (stCtx == NULL) {
        dlclose(handle);
        return (struct ForignProgram_CTX){.isError = 1};
    }
    stCtx->entry = forignRun;
    thrd_t* thread = malloc(sizeof(*thread));
    nat_SafeThread* sf_thrd = malloc(sizeof(*sf_thrd));
    sf_thrd->thread = thread;
    stCtx->self = sf_thrd;
    struct nat_SafeThread_CreateArg createArg = {
        .thread = sf_thrd,
        .callback = SpawnSafeThreadWithEntry,
        .ctx = stCtx,
    };
    if (nat_threadCreate(&createArg) != 0) {
        free(stCtx);
        dlclose(handle);
        return (struct ForignProgram_CTX){.isError = 1};
    }

    return (struct ForignProgram_CTX){
        .prog_handle = handle,
        .main_ForignThread = sf_thrd,
    };
}

int native_run(void) {
#define SERVER_PORT 8000
#define SERVER_IP   "127.0.0.1"

    mtx_init(&nat_SyscallMutex, mtx_plain);

    WOLFSSL_CTX* ctx = NULL;
    WOLFSSL*     ssl = NULL;

    /* 1. Initialize wolfSSL library */
    wolfSSL_Init();

    /* 2. Create a WOLFSSL_CTX context using TLS 1.3 client method */
    if ((ctx = wolfSSL_CTX_new(wolfTLSv1_3_client_method())) == NULL) {
        fprintf(stderr, "wolfSSL_CTX_new error.\n");
        return -1;
    }
    /* wolfSSL clients verify the peer by default, so load the CA that signed
       the testBench server's certificate; without it the handshake fails with
       an "unknown ca" alert.  We are a pure client and present no certificate
       of our own -- the server does not request one. */
    if (wolfSSL_CTX_load_verify_locations(ctx, "certs/ca-cert.pem", NULL) != WOLFSSL_SUCCESS) {
        fprintf(stderr, "Error loading ca-cert.pem\n");
        return -1;
    }
    /* 4. Create a standard TCP socket and connect to the testBench server. */
    int sockfd;
    struct sockaddr_in servAddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &servAddr.sin_addr) != 1) {
        fprintf(stderr, "invalid server address %s\n", SERVER_IP);
        return -1;
    }
    if (connect(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) != 0) {
        perror("connect");
        return -1;
    }
    printf("Connected to %s:%d\n", SERVER_IP, SERVER_PORT);

    /* 5. Create a WOLFSSL object and associate it with the socket */
    if ((ssl = wolfSSL_new(ctx)) == NULL) {
        fprintf(stderr, "wolfSSL_new error.\n");
        return -1;
    }
    wolfSSL_set_fd(ssl, sockfd);

    int fd = memfd_create("incomming plugin", 0);
    if (fd == -1) {
        perror("memfd_create");
    }

    /* 6. Perform TLS Handshake */
    if (wolfSSL_connect(ssl) != WOLFSSL_SUCCESS) {
        fprintf(stderr, "wolfSSL_connect error\n");
    } else {
        /* 7. Ask the server for a program, then stream the reply. */
        const char* request = "GET libAidan\n";
        if (wolfSSL_write(ssl, request, (int)strlen(request)) <= 0) {
            fprintf(stderr, "wolfSSL_write error\n");
        }
        while (true) {
            char reply[64 << 10] = {0};

            int read = wolfSSL_read(ssl, reply, sizeof(reply) - 1);
            if (read == 0) break;

            if (write(fd, reply, (size_t)read) != (ssize_t)read) {
                perror("write");
                close(fd);
            }
            printf("received %d bytes\n", read);
        }
    }

    /* 8. Cleanup and resources release */
    wolfSSL_free(ssl);
    wolfSSL_CTX_free(ctx);
    wolfSSL_Cleanup();
    close(sockfd);
    nat_SyscallQueue = malloc(sizeof(*nat_SyscallQueue) +
            (sizeof(struct nat_Syscall_Request*) * 20));
    assert(nat_SyscallQueue);
    *nat_SyscallQueue = (struct nat_SyscallQueue){
        .cap = 20,
        .len = 0,
        .nextUp = 0,
        .nextFree = 0,
    };
    struct ForignProgram_CTX forignProgramCtx = runForignProgram(fd);
    printf("running program!\n");
    fflush(stdout);
    if (forignProgramCtx.isError) {
        printf("Error");
    }
    while (forignProgramCtx.main_ForignThread->isRunning) {
        mtx_lock(&nat_SyscallMutex);
        while (nat_SyscallQueue->len > 0) {
            nat_runSyscallRequest(nat_getNextSyscall());
        }
        mtx_unlock(&nat_SyscallMutex);
        sched_yield();
    }

    thrd_join(*forignProgramCtx.main_ForignThread->thread, NULL);

    mtx_destroy(&nat_SyscallMutex);
    free(forignProgramCtx.main_ForignThread->thread);
    free(forignProgramCtx.main_ForignThread);
    dlclose(forignProgramCtx.prog_handle);


    return EXIT_SUCCESS;
}
