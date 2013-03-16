#!/bin/sh
#

set -e

# create version file
./mkversion.sh $@

# run autotools
autoreconf -i
