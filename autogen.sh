#!/bin/bash
./mylcm_gen.sh
libtoolize --force
aclocal
autoheader
automake --force-missing --add-missing
autoconf
./configure CFLAGS="-I/usr/local/include -I/usr/include -I/usr/include/python2.7 -lpython2.7" LDFLAGS="-L/usr/local/lib"
##./configure CFLAGS="-I/usr/local/include -I/usr/include -I/usr/include/python3.4 -lpython3.4" LDFLAGS="-L/usr/local/lib -L/usr/local/lib/python3.4 -L/usr/lib/python3.4/config-3.4m-x86_64-linux-gnu/"
bash ./mymake.sh
