#!/bin/bash
libtoolize --force
aclocal
autoheader
automake --force-missing --add-missing
autoconf
./configure CFLAGS="-I/usr/local/include" LDFLAGS="-L/usr/local/lib"
bash ./mymake.sh
