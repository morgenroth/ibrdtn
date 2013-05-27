#!/bin/sh
#

set -e

# extract the latest version
VERSION=$(cat daemon/debian/changelog | head -n 1 | sed 's/[a-z\-]* (\(.*\)) .*$/\1/')

# ... and set it as default
MAJOR=$(echo ${VERSION} | sed 's/^\([0-9]*\).[0-9]*.[0-9]*$/\1/')
MINOR=$(echo ${VERSION} | sed 's/^[0-9]*.\([0-9]*\).[0-9]*$/\1/')
MICRO=$(echo ${VERSION} | sed 's/^[0-9]*.[0-9]*.\([0-9]*\)$/\1/')

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
echo "$MAJOR.$MINOR.$MICRO" > version.inc
