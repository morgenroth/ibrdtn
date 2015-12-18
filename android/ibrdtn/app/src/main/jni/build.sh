#!/bin/sh
#

OPENSSL_COMMIT="01e473f0a6320fafec82e3cc015bb2653879ef51"
LIBNL_COMMIT="7e32da396adfe6b58b23641dacb1887f5855ff9c"

set -e

echo "Generating symlinks to IBR-DTN sources..."
echo "-----------------------------------------"
[ ! -e ibrcommon ] && ln -s ../../../../../../ibrcommon/ ibrcommon
[ ! -e ibrdtn ] && ln -s ../../../../../../ibrdtn/ibrdtn/ ibrdtn
[ ! -e dtnd ] && ln -s ../../../../../../ibrdtn/daemon/ dtnd

echo ""
echo "Cloning external git sources used in IBR-DTN (libnl and openssl)..."
echo "-------------------------------------------------------------------"

# check if the existing directory has the right revision
if [ -e nl-3 ]; then
  cd nl-3; REV=$(git rev-parse HEAD | tr -d '\n'); cd ..
  if [ "${REV}" != "${LIBNL_COMMIT}" ]; then
    rm -rf nl-3
  fi
fi
if [ ! -d nl-3 ]; then
  git clone git://github.com/ibrdtn/libnl-3-android.git nl-3
  cd nl-3
  git checkout --detach ${LIBNL_COMMIT}
  cd ..
fi

# check if the existing directory has the right revision
if [ -e openssl ]; then
  cd openssl; REV=$(git rev-parse HEAD~1 | tr -d '\n'); cd ..
  if [ "${REV}" != "${OPENSSL_COMMIT}" ]; then
    rm -rf openssl
  fi
fi
if [ ! -d openssl ]; then
  git clone git://github.com/ibrdtn/openssl-android.git openssl
  cd openssl
  git checkout --detach ${OPENSSL_COMMIT}
  git am < ../0001-renamed-crypto-library.patch
  cd ..
fi

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
	make clean
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
ndk-build -j
