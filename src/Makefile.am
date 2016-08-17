## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libromancalc.la
libromancalc_la_SOURCES = romancalc.c romancalc.h

bin_PROGRAMS = main
main_SOURCES = main.c
main_LDADD = libromancalc.la
