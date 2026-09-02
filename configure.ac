AC_PREREQ([2.69])
AC_INIT([native], [0.1.0], [aidankeefe486@gmail.com])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.16 foreign subdir-objects -Wall -Werror])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_AR
AC_PROG_RANLIB

# Checks for libraries.
# wolfSSL is required by src/native.c.  Prefer pkg-config (wolfssl.pc), and
# fall back to a manual search for the common /usr/local install that ships
# its pkgconfig dir off the default PKG_CONFIG_PATH.
PKG_CHECK_MODULES([WOLFSSL], [wolfssl], [have_wolfssl=yes], [have_wolfssl=no])
AS_IF([test "x$have_wolfssl" = xno],
  [native_save_CPPFLAGS="$CPPFLAGS"
   native_save_LIBS="$LIBS"
   CPPFLAGS="$CPPFLAGS -I/usr/local/include"
   LIBS="$LIBS -L/usr/local/lib"
   AC_CHECK_HEADER([wolfssl/ssl.h], [],
     [AC_MSG_ERROR([wolfSSL headers not found; install wolfSSL or set PKG_CONFIG_PATH])])
   AC_CHECK_LIB([wolfssl], [wolfSSL_Init], [],
     [AC_MSG_ERROR([libwolfssl not found; install wolfSSL or set PKG_CONFIG_PATH])])
   WOLFSSL_CFLAGS="-I/usr/local/include"
   WOLFSSL_LIBS="-L/usr/local/lib -lwolfssl"
   CPPFLAGS="$native_save_CPPFLAGS"
   LIBS="$native_save_LIBS"])
AC_SUBST([WOLFSSL_CFLAGS])
AC_SUBST([WOLFSSL_LIBS])



# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([strdup])

# Optional debug build: ./configure --enable-debug
AC_ARG_ENABLE([debug],
  [AS_HELP_STRING([--enable-debug], [build with debug symbols and no optimization])],
  [enable_debug=$enableval],
  [enable_debug=no])
AS_IF([test "x$enable_debug" = "xyes"],
  [CFLAGS="$CFLAGS -g3 -O0 -DDEBUG"])
AM_CONDITIONAL([DEBUG], [test "x$enable_debug" = "xyes"])

CFLAGS="$CFLAGS -std=c23 -lseccomp"
# Check if the compiler supports stripping the source prefix from __FILE__.
# Done inline rather than with AX_CHECK_COMPILE_FLAG so that no dependency on
# autoconf-archive is needed.  The concrete path is left to make, so that
# abs_top_srcdir is used rather than the possibly-relative $srcdir.
MACRO_PREFIX_MAP_CFLAGS=
AC_CACHE_CHECK([whether $CC accepts -fmacro-prefix-map],
  [native_cv_cflags_macro_prefix_map],
  [native_save_CFLAGS="$CFLAGS"
   CFLAGS="$CFLAGS -Werror -fmacro-prefix-map=/native-conftest/=."
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],
     [native_cv_cflags_macro_prefix_map=yes],
     [native_cv_cflags_macro_prefix_map=no])
   CFLAGS="$native_save_CFLAGS"])
AS_IF([test "x$native_cv_cflags_macro_prefix_map" = xyes],
  [MACRO_PREFIX_MAP_CFLAGS='-fmacro-prefix-map=$(abs_top_srcdir)/='])
AC_SUBST([MACRO_PREFIX_MAP_CFLAGS])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  tests/Makefile
])

AC_OUTPUT

AC_MSG_NOTICE([

  $PACKAGE_NAME $PACKAGE_VERSION

  prefix .......... $prefix
  compiler ........ $CC
  CFLAGS .......... $CFLAGS
  debug ........... $enable_debug
])
