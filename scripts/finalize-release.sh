#!/bin/bash
#

MAINDIR="ibrdtn/daemon"
DIRS="ibrcommon ibrdtn/ibrdtn ibrdtn/daemon ibrdtn/tools"
WORKSPACE=$(pwd)

export EMAIL="morgenroth@ibr.cs.tu-bs.de"
export NAME="Johannes Morgenroth"

##############################################################################

if [ ! -d "${MAINDIR}" ]; then
    echo "Please execute the script in the root directory of the repository."
    exit 1
fi

cd ${WORKSPACE}/${MAINDIR}

VERSION=`cat debian/changelog | head -n 1 | sed 's/[a-z\-]* (\(.*\)) .*$/\1/'`

# ... and set it as default
MAJOR=`echo ${VERSION} | sed 's/^\([0-9]*\).[0-9]*.[0-9]*$/\1/'`
MINOR=`echo ${VERSION} | sed 's/^[0-9]*.\([0-9]*\).[0-9]*$/\1/'`
MICRO=`echo ${VERSION} | sed 's/^[0-9]*.[0-9]*.\([0-9]*\)$/\1/'`

for DIR in ${DIRS}; do
    cd ${WORKSPACE}/${DIR}

    echo "finalize changelog (${DIR})"
    dch -v "${MAJOR}.${MINOR}.${MICRO}" -r

    git add debian/changelog
done

git commit -m "Release ${MAJOR}.${NEXT_MINOR}.${MICRO}"

# create new release tag
git tag -a release/${MAJOR}.${MINOR}.${MICRO} -m "Release ${MAJOR}.${MINOR}.${MICRO}"

let NEXT_MINOR=${MINOR}+1

for DIR in ${DIRS}; do
    cd ${WORKSPACE}/${DIR}

    dch -v "${MAJOR}.${NEXT_MINOR}.${MICRO}" -D UNRELEASED "Development revision"
    git add debian/changelog
done

git commit -m "Introduce development revision ${MAJOR}.${NEXT_MINOR}.${MICRO}"

