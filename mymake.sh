#!/bin/bash
if [ -e src/*.pyc ]
then
	rm src/*.pyc
fi
if [ -e tests/check_romancalc.log ]
then
	rm tests/check_romancalc.log
fi

./mylcm_gen.sh

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
