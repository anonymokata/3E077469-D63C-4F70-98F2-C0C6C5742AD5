# NOTE: this was borrowed from the sample code
# of the https://libcheck.github.io/check/
# Process this file with autoconf to produce a configure script.

# Prelude.
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_COMMANDS([mkdir], [$MKDIR_P/m4])
AC_PREREQ([2.59])
AC_INIT([romancalc], [0.3], [author at comments])

# unique source file --- primitive safety check 
AC_CONFIG_SRCDIR([src/romancalc.c])

# place to put some extra build scripts installed
AC_CONFIG_AUX_DIR([build-aux])

# fairly severe build strictness
# change foreign to gnu or gnits to comply with gnu standards
AM_INIT_AUTOMAKE([-Wall -Werror foreign 1.9.6])

# Checks for programs.
AM_PROG_AR
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.

# This macro is defined in check.m4 and tests if check.h and
# libcheck.a are installed in your system. It sets CHECK_CFLAGS and
# CHECK_LIBS accordingly.  
#  AM_PATH_CHECK([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
# AM_PATH_CHECK()
# AM_PATH_CHECK depredicated https://sourceforge.net/p/check/mailman/message/2751663/
PKG_CHECK_MODULES([CHECK], [check >= 0.9.4],,[AC_MSG_RESULT([no, testing is disabled])])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])
##PKG_CHECK_MODULES([CHECK], [check],[have_check=yes], [have_check=no])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC

# Output files
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tests/Makefile])

AC_OUTPUT
