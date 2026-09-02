AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)
AM_CFLAGS = -Wall -Wextra $(MACRO_PREFIX_MAP_CFLAGS)
AM_CPPFLAGS += $(WOLFSSL_CFLAGS)

check_PROGRAMS = test_native
test_native_SOURCES = test_native.c
test_native_LDADD = $(top_builddir)/src/libnative.a $(WOLFSSL_LIBS)

TESTS = $(check_PROGRAMS)
