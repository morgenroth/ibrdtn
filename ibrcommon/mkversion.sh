#!/bin/bash
#

# extract the latest version
VERSION=`cat debian/changelog | head -n 1 | sed 's/[a-z\-]* (\(.*\)) .*$/\1/'`

# ... and set it as default
MAJOR=`echo ${VERSION} | sed 's/^\([0-9]*\).[0-9]*.[0-9]*$/\1/'`
MINOR=`echo ${VERSION} | sed 's/^[0-9]*.\([0-9]*\).[0-9]*$/\1/'`
MICRO=`echo ${VERSION} | sed 's/^[0-9]*.[0-9]*.\([0-9]*\)$/\1/'`

# create a version.inc file
if [ -n "$1" ] && [ -n "$2" ]; then
	MAJOR=$1
	MINOR=$2
	
	if [ -n "$3" ]; then
		MICRO=$3
	fi
fi

echo "set version to ${MAJOR}.${MINOR}.${MICRO}"

# write version to version.inc file
echo "m4_define([PKG_FULL_VERSION], [$MAJOR.$MINOR.$MICRO])" > version.m4
echo "m4_define([PKG_LIB_VERSION], [1:0:0])" >> version.m4
echo "m4_define([PKG_MAJOR_VERSION], [$MAJOR])" >> version.m4
echo "m4_define([PKG_MINOR_VERSION], [$MINOR])" >> version.m4
echo "m4_define([PKG_MICRO_VERSION], [$MICRO])" >> version.m4
