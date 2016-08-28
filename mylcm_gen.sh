#!/bin/bash
cd src
#if files don't exist then make them
if [ ! -e exlcm/__init__.py ]  ||  [ ! -e exlcm/rn_packet_t.py ] || [ rn_packet.lcm -nt exlcm/__init__.py ] || [ rn_packet.lcm -nt exlcm/rn_packet_t.py ]
then
echo regenerating lcm python interface
lcm-gen -p rn_packet.lcm
fi
cd ../tests

# if the files dont exist or the source file is newer thatn then the output file
if [ ! -e  exlcm_rn_packet_t.c ]  ||  [ ! -e  exlcm_rn_packet_t.h ] || [ ../src/rn_packet.lcm -nt exlcm_rn_packet_t.c ] || [ ../src/rn_packet.lcm -nt exlcm_rn_packet_t.h ]
then

echo regenerating lcm c interface
lcm-gen -c ../src/rn_packet.lcm
fi
cd ..
