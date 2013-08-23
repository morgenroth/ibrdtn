#!/bin/sh
#

VERSION="0.11.0"
SOURCES="/d/sources"
DESTDIR="/d/apps/ibrdtn"

CONFIGURE_OPTIONS="--enable-win32 --prefix=${DESTDIR}"

ibrcommon_PATH="${SOURCES}/ibrcommon-${VERSION}"
ibrdtn_PATH="${SOURCES}/ibrdtn-${VERSION}"

export ibrcommon_CFLAGS="-I${ibrcommon_PATH}"
export ibrcommon_LIBS="-L${ibrcommon_PATH}/ibrcommon/.libs -librcommon"

export ibrdtn_CFLAGS="${ibrcommon_CFLAGS} -I${ibrdtn_PATH}"
export ibrdtn_LIBS="${ibrcommon_LIBS} -L${ibrdtn_PATH}/ibrdtn/.libs -librdtn"

cd ibrcommon-${VERSION}
./configure ${CONFIGURE_OPTIONS}
make
cd ..

cd ibrdtn-${VERSION}
./configure ${CONFIGURE_OPTIONS}
make
cd ..

cd ibrdtnd-${VERSION}
./configure ${CONFIGURE_OPTIONS} --enable-ntservice
make

