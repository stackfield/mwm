#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(mwm, 1.0, [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([stdlib.h sys/param.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([bzero gettimeofday mblen memmove memset putenv setlocale strchr strncasecmp strpbrk strrchr strstr])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_PROG_RANLIB

AC_CHECK_HEADERS([X11/Xlib.h],,AC_MSG_ERROR([X11 Header not found]))
AC_CHECK_HEADERS([Xm/Xm.h],,AC_MSG_ERROR([Motif Header not found]))
AC_CHECK_LIB([X11], [XOpenDisplay],, AC_MSG_ERROR([X11 Library not found]))
AC_CHECK_LIB([Xm], [XmCreateMainWindow],, AC_MSG_ERROR([Motif Library not found]))

CFLAGS="$CFLAGS $X11_CFLAGS $XM_CFLAGS"
LIBS="$LIBS $X11_LIBS $XM_LIBS"
AC_OUTPUT
