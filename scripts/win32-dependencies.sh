#!/bin/bash

for F in `find -name *.exe -or -name *.dll`; do echo $F; objdump -p $F | grep 'DLL-Name:'; done
