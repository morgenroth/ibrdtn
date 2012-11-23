#!/bin/bash
#

rm -rf ibrdtn-win32
mkdir -p ibrdtn-win32

FILES="./libs/pthreads-w32/pthreadGC2.dll `find ./win32-inst/bin -name *.dll` `find ./win32-inst/sbin -name *.exe`"

for F in $FILES; do
cp $F ibrdtn-win32/
done

rm ibrdtn-win32.zip
zip -r ibrdtn-win32.zip ibrdtn-win32/
rm -rf ibrdtn-win32/

