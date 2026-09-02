#ifndef NATIVE_H
#define NATIVE_H

/* Connect to the local TLS server, stream the shared object into an
   anonymous in-memory file, then dlopen() it and invoke its `run` symbol.
   Returns a process exit status (EXIT_SUCCESS on success, EXIT_FAILURE or a
   negative value on error). */
int native_run(void);

/* impliment this function */
typedef int(*ProgramEntry)(void);
extern ProgramEntry run;
#endif /* NATIVE_H */
