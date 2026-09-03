AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)
AM_CFLAGS = -Wall -Wextra $(MACRO_PREFIX_MAP_CFLAGS)
AM_CPPFLAGS += $(WOLFSSL_CFLAGS)

# Core logic lives in a convenience library so tests can link against it.
noinst_LIBRARIES = libnative.a
libnative_a_SOURCES = native.c \
					  safeThreads.c \
					  logging.c \
					  syscallQueue.c


bin_PROGRAMS = native
native_SOURCES = main.c
native_LDADD = libnative.a $(WOLFSSL_LIBS)
