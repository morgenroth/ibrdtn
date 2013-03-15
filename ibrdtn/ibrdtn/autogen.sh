#!/bin/sh
#

set -e

# create version file
. mkversion.sh $@

# run libtool
libtoolize -i -c

# run autotools
aclocal
automake --add-missing -c
autoreconf -i
