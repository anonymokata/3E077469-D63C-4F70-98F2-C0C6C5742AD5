## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4
TESTS = check_romancalc
check_PROGRAMS = check_romancalc
check_romancalc_SOURCES = check_romancalc.c $(top_builddir)/src/romancalc.h
check_romancalc_CFLAGS = @CHECK_CFLAGS@
check_romancalc_LDADD = $(top_builddir)/src/libromancalc.la @CHECK_LIBS@
