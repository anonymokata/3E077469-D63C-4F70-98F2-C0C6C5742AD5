#!/bin/bash
libtoolize --force
aclocal
autoheader
automake --force-missing --add-missing
autoconf
./configure CFLAGS="-I/usr/local/include -I/usr/include -I/usr/include/python2.7 -lpython2.7" LDFLAGS="-L/usr/local/lib"
bash ./mymake.sh
