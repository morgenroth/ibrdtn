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

let PREV_MINOR=${MINOR}-1
let NEXT_MINOR=${MINOR}+1

for DIR in ${DIRS}; do
    cd ${WORKSPACE}/${DIR}

    git log --oneline release/${MAJOR}.${PREV_MINOR}.${MICRO}..HEAD . | cut -d' ' -f2- \
    | while read line; do
        #escaped_line=$(echo -n ${line} | sed 's/-/\\-/g')
        #escaped_line=$(echo -n ${escaped_line} | sed 's/\"/\\\"/g')
        echo "Add[${DIR}]: $line"
        dch -v "${MAJOR}.${NEXT_MINOR}.${MICRO}" -D UNRELEASED "$line"
    done
done



