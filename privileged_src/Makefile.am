AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)
AM_CFLAGS = -Wall -Wextra $(MACRO_PREFIX_MAP_CFLAGS)
AM_CPPFLAGS += $(WOLFSSL_CFLAGS)

# Core logic lives in a convenience library so tests can link against it.
noinst_LIBRARIES = libnative.a
libnative_a_SOURCES = native.c \
					  safeThreads.c \
					  logging.c \
					  runSyscall.c

bin_PROGRAMS = native

# The client-side API is compiled straight into the daemon rather than linked
# as an archive: archive members the daemon never references would be dropped,
# and plugins need every one of these symbols at dlopen() time.
native_SOURCES = main.c \
				 ../src/loggingInterface.c \
				 ../src/safeThreadInterface.c \
				 ../src/syscallQueue.c

# -rdynamic puts those symbols in .dynsym, so a plugin dlopen()ed into this
# process resolves the client API against the daemon instead of carrying it.
native_LDFLAGS = -rdynamic

native_LDADD = libnative.a $(WOLFSSL_LIBS)
