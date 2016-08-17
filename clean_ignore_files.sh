#!/bin/bash
echo
echo
echo '**************** begin of clean_ignore_file.sh *********************'
echo '********************************************************************'
echo   NOTE: 8/7/2016  this file wipes directories back to basic so that
echo       make, automake, autoconf, and autoreconf can be tested
echo '********************************************************************'
echo '********************************************************************'
echo
echo
rm    aclocal.m4
rm -R autom4te.cache
rm -R build-aux
rm    config.h
rm    config.h.in
rm    config.log
rm    config.status
rm    configure
rm    libtool
##mkdir m4
##rm -R m4/*
rm -R m4
##mkdir m4
rm    Makefile
rm    Makefile.in
rm -R src/.deps
rm -R src/.libs
rm    src/*.o
rm    src/*.lo
rm    src/libromancalc.la
rm    src/main
rm    src/Makefile
rm    src/Makefile.in
rm    stamp-h1
rm -R tests/.deps
rm -R tests/.libs
rm    tests/*.o
#rm    tests/*.lo
rm    tests/Makefile
rm    tests/Makefile.in
rm    tests/check_romancalc.trs
echo
echo '***************** end of clean_ignore_file.sh *********************'
