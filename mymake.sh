#!/bin/bash
rm src/*.pyc
rm tests/check_romancalc.log
sleep 2
make
sleep 2
cd tests
sleep 2
make check_romancalc
sleep 2
cd ..
sleep 2
make check
echo "to see details of results pass/failure"
echo "at the prompt type \"cat tests/check_romancalc.log\""
