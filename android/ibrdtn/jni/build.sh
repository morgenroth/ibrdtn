echo "Generating symlinks to IBR-DTN sources..."
echo "-----------------------------------------"
ln -s ../../../ibrcommon/ ibrcommon
ln -s ../../../ibrdtn/ibrdtn/ ibrdtn
ln -s ../../../ibrdtn/daemon/ dtnd

echo ""
echo "Cloning external git sources used in IBR-DTN (libnl and openssl)..."
echo "-------------------------------------------------------------------"
git clone git://github.com/ibrdtn/libnl-3-android.git nl-3
cd nl-3
git fetch --tags
git checkout origin
git checkout 7e32da396adfe6b58b23641dacb1887f5855ff9c
cd ..
git clone git://github.com/guardianproject/openssl-android.git openssl
cd openssl
git fetch --tags
git checkout origin
git checkout 327836b3b18784d5da25ffe904dc37b6cf368c81
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
echo "Building IBR-DTN with Android NDK..."
echo "------------------------------------"
ndk-build