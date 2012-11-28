#!/bin/bash -xe
#

DESTDIR="$(pwd)/win32-inst"

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
make clean
make LFLAGS="${STATIC_OPTS}" CROSS=${CROSSARCH}- clean GC-inlined
mkdir -p ${DESTDIR}/bin
cp `find -name *.dll` ${DESTDIR}/bin
cd ../..

cd ibrcommon
make clean
bash autogen.sh
./configure --build=x86_64-unknown-linux-gnu --host=${CROSSARCH} --with-mingw32-pthread=/home/morgenro/sources/ibrdtn-windows.git/libs/pthreads-w32 --disable-netlink --enable-win32 --prefix=${DESTDIR} --enable-shared --enable-static
make -j
make install
cd ..

cd ibrdtn/ibrdtn
make clean
bash autogen.sh
./configure --build=x86_64-unknown-linux-gnu --host=${CROSSARCH} --enable-win32 --prefix=${DESTDIR} --enable-shared --enable-static
make -j
make install
cd ..

cd daemon
make clean
bash autogen.sh
./configure --build=x86_64-unknown-linux-gnu --host=${CROSSARCH} --enable-win32 --enable-ntservice --disable-dtndht --disable-libdaemon --prefix=${DESTDIR}
make -j
make install
cd ..
cd ..

# strip binaries
BINARIES="sbin/dtnd.exe sbin/dtnserv.exe"
for BINARY in ${BINARIES}; do
    ${CROSSARCH}-strip ${DESTDIR}/${BINARY}
done

