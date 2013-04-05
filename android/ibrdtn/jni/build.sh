#!/bin/sh
#

set -e

echo "Generating symlinks to IBR-DTN sources..."
echo "-----------------------------------------"
[ ! -e ibrcommon ] && ln -s ../../../ibrcommon/ ibrcommon
[ ! -e ibrdtn ] && ln -s ../../../ibrdtn/ibrdtn/ ibrdtn
[ ! -e dtnd ] && ln -s ../../../ibrdtn/daemon/ dtnd

echo ""
echo "Cloning external git sources used in IBR-DTN (libnl and openssl)..."
echo "-------------------------------------------------------------------"
[ ! -e nl-3 ] && git clone git://github.com/ibrdtn/libnl-3-android.git nl-3
cd nl-3
git fetch --tags
git checkout origin
git checkout 7e32da396adfe6b58b23641dacb1887f5855ff9c
cd ..
[ ! -e openssl ] && git clone git://github.com/dschuermann/openssl-android.git openssl
cd openssl
git fetch --tags
git checkout origin
git checkout 8c6a9abf76767407afd652ed65fba32620c38e04
cd ..

echo ""
echo "Generating Android.mk files using Autotools and Androgenizer..."
echo "---------------------------------------------------------------"
for SOURCE in "ibrcommon" "ibrdtn" "dtnd"
do
	echo ""
	echo "Generating Android.mk files for $SOURCE"
	echo "---------------------------------------"
	cd $SOURCE
	./autogen.sh
	./configure --enable-android
	make
	cd ..
done

echo ""
echo "Generate SWIG wrapper classes..."
echo "--------------------------------"
rm -Rf ../src/de/tubs/ibr/dtn/swig
mkdir -p ../src/de/tubs/ibr/dtn/swig
swig -c++ -java -package de.tubs.ibr.dtn.swig -verbose -outdir ../src/de/tubs/ibr/dtn/swig/ -o android-glue/SWIGWrapper.cpp android-glue/swig.i 

echo ""
echo "Building IBR-DTN with Android NDK..."
echo "------------------------------------"
ndk-build
