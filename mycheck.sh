#!/bin/bash
rm src/*.pyc
sleep 2
cd tests
sleep 2
make check_romancalc
sleep 2
cd ..
sleep 2
make check
sleep 2
cat tests/check_romancalc.log
