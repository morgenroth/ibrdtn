#!/bin/bash -xe
#

DESTDIR="$(pwd)/linux-inst"

# set defaults
[ -z "${CLEAN}" ] && CLEAN=1
[ -z "${STRIP}" ] && STRIP=1
[ -z "${DEBUG}" ] && DEBUG=0

if [ ${DEBUG} -eq 1 ]; then
    CXXFLAGS="-ggdb -g3 -Wall"
    STRIP=0
fi

# clean destdir
rm -rf ${DESTDIR}

cd ibrcommon
if [ ${CLEAN} -eq 1 ]; then
    make clean
    bash autogen.sh
    ./configure --prefix=${DESTDIR} --with-openssl --with-curl --with-lowpan --with-sqlite --with-dtnsec --with-compression --with-xml --with-tls --with-cppunit --with-dht
fi
make -j
make install
cd ..

cd ibrdtn/ibrdtn
if [ ${CLEAN} -eq 1 ]; then
    make clean
    bash autogen.sh
    ./configure --prefix=${DESTDIR} --with-openssl --with-curl --with-lowpan --with-sqlite --with-dtnsec --with-compression --with-xml --with-tls --with-cppunit --with-dht
fi
make -j
make install
cd ..

cd daemon
if [ ${CLEAN} -eq 1 ]; then
    make clean
    bash autogen.sh
    ./configure --prefix=${DESTDIR} --with-openssl --with-curl --with-lowpan --with-sqlite --with-dtnsec --with-compression --with-xml --with-tls --with-cppunit --with-dht
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

