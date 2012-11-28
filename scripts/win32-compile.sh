#!/bin/bash -xe
#

DESTDIR="$(pwd)/win32-inst"

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

#CROSSARCH="x86_64-w64-mingw32"
CROSSARCH="i686-w64-mingw32"
STATIC_OPTS="-static-libstdc++ -static-libgcc"

# install pthreads
if [ ! -e "libs/pthreads-w32" ]; then
    PKG_NAME="pthreads-w32-2-9-1-release"
    PKG_URL="ftp://sourceware.org/pub/pthreads-win32/${PKG_NAME}.tar.gz"
    mkdir -p libs/
    cd libs
    wget -O - "${PKG_URL}" | tar xvz
    ln -s ${PKG_NAME} pthreads-w32
    cd ..
fi

cd libs/pthreads-w32
[ ${CLEAN} -eq 1 ] && make clean
make LFLAGS="${STATIC_OPTS}" CROSS=${CROSSARCH}- clean GC-inlined
mkdir -p ${DESTDIR}/bin
cp `find -name *.dll` ${DESTDIR}/bin
cd ../..

cd ibrcommon
if [ ${CLEAN} -eq 1 ]; then
    make clean
    bash autogen.sh
    ./configure --build=x86_64-unknown-linux-gnu --host=${CROSSARCH} --with-mingw32-pthread=/home/morgenro/sources/ibrdtn-windows.git/libs/pthreads-w32 --disable-netlink --enable-win32 --prefix=${DESTDIR} --enable-shared --enable-static
fi
make -j
make install
cd ..

cd ibrdtn/ibrdtn
if [ ${CLEAN} -eq 1 ]; then
    make clean
    bash autogen.sh
    ./configure --build=x86_64-unknown-linux-gnu --host=${CROSSARCH} --enable-win32 --prefix=${DESTDIR} --enable-shared --enable-static
fi
make -j
make install
cd ..

cd daemon
if [ ${CLEAN} -eq 1 ]; then
    make clean
    bash autogen.sh
    ./configure --build=x86_64-unknown-linux-gnu --host=${CROSSARCH} --enable-win32 --enable-ntservice --disable-dtndht --disable-libdaemon --prefix=${DESTDIR}
fi
make -j
make install
cd ..
cd ..

if [ ${STRIP} -eq 1 ]; then
# strip binaries
BINARIES="sbin/dtnd.exe sbin/dtnserv.exe"
for BINARY in ${BINARIES}; do
    ${CROSSARCH}-strip ${DESTDIR}/${BINARY}
done
fi

