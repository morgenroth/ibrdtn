#!/bin/sh
#
# This script generate all missing files.
#
#

set -e

LOCALDIR=`pwd`
SUBDIRS="ibrcommon ibrdtn daemon tools"

# create ibrcommon sym link
if [ ! -e "ibrcommon" ]; then
    ln -s ../ibrcommon ibrcommon
fi

for DIR in $SUBDIRS; do
	echo "## enter directory $DIR ##"
	cd ${LOCALDIR}/${DIR}

	# create m4 directory
	mkdir -p m4

	if [ -e "autogen.sh" ]; then
		echo "## run autogen.sh script ##"
		./autogen.sh $@
	fi 
done

echo "## run autoreconf in the main directory ##"
cd ${LOCALDIR}

# create m4 directory
mkdir -p m4

# create version file
./mkversion.sh $@

# run autotools
autoreconf -i
