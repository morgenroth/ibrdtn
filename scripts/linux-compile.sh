#!/bin/bash -xe
#

DESTDIR="$(pwd)/linux-inst"

# set defaults
[ -z "${CLEAN}" ] && CLEAN=1
[ -z "${STRIP}" ] && STRIP=1
[ -z "${DEBUG}" ] && DEBUG=0
[ -z "${PROFILE}" ] && PROFILE=0

CXXFLAGS=""

if [ ${DEBUG} -eq 1 ]; then
    CXXFLAGS+=" -ggdb -g3 -Wall"
    STRIP=0
fi

if [ ${PROFILE} -eq 1 ]; then
    CXXFLAGS+=" -pg"
fi

# clean destdir
rm -rf ${DESTDIR}

cd ibrcommon
if [ ${CLEAN} -eq 1 ]; then
    [ -f Makefile ] && make clean
    bash autogen.sh
    ./configure --prefix=${DESTDIR} --with-openssl --with-lowpan
fi
make -j
make install
cd ..

cd ibrdtn/ibrdtn
if [ ${CLEAN} -eq 1 ]; then
    [ -f Makefile ] && make clean
    bash autogen.sh
    ./configure --prefix=${DESTDIR} --with-dtnsec --with-compression
fi
make -j
make install
cd ..

cd daemon
if [ ${CLEAN} -eq 1 ]; then
    [ -f Makefile ] && make clean
    bash autogen.sh
    ./configure --prefix=${DESTDIR} --with-curl --with-lowpan --with-sqlite --with-dtnsec --with-compression --with-tls --with-cppunit 
fi
make -j
make install
cd ..
cd ..

if [ ${STRIP} -eq 1 ]; then
# strip binaries
BINARIES="sbin/dtnd"
for BINARY in ${BINARIES}; do
    strip ${DESTDIR}/${BINARY}
done
fi

