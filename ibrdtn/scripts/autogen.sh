#!/bin/bash
#
cd ibrcommon
bash autogen.sh
cd ..
autoheader configure.in
autoconf --include=include
