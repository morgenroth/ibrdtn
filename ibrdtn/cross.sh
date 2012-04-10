#!/bin/bash
#

# define $BUILDROOT variable to build against a buildroot toolchain

# get architecture
if [ -e ".cross-arch" ]; then
	ARCH=`cat .cross-arch`
fi

if [ -e ".cross-host" ]; then
	HOST=`cat .cross-host`
fi

# create parameters
export AR="$ARCH-ar"
export AS="$ARCH-gcc"
export LD="$ARCH-ld"
export NM="$ARCH-nm"
export CC="$ARCH-gcc"
export GCC="$ARCH-gcc"
export CXX="$ARCH-g++"
export RANLIB="$ARCH-ranlib"
export STRIP="$ARCH-strip"
export OBJCOPY="$ARCH-objcopy"
export OBJDUMP="$ARCH-objdump"
export CC="$ARCH-gcc"
export CXX="$ARCH-g++"
export CPPFLAGS="-I/usr/$ARCH/include/c++"

if [ -n "$BUILDROOT" ]; then
	export CPPFLAGS="-I$BUILDROOT/output/staging/usr/include \
		-I$BUILDROOT/output/staging/usr/include/c++ \
		-I$BUILDROOT/output/toolchain/uClibc_dev/usr/include \
		-I$BUILDROOT/output/toolchain/linux/include"

	export LDFLAGS="-L$BUILDROOT/output/staging/usr/lib \
		-L$BUILDROOT/output/toolchain/uClibc_dev/usr/lib"

	export PATH="$PATH:$BUILDROOT/output/staging/usr/bin"
fi

if [ "$1" == "compile" ]; then
	make
elif [ "$1" == "configure" ]; then
	export ac_cv_func_malloc_0_nonnull=yes
	./configure CXXFLAGS="-g3 -ggdb -Wall -Wextra" --target=$ARCH --host=$ARCH --build=$HOST --prefix=/opt/ibrdtn-$ARCH
elif [ "$1" == "install" ]; then
	make install
else
	echo "$1" > .cross-arch
	echo "$2" > .cross-host
fi

