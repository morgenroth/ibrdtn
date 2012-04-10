#!/bin/bash
#

TESTS=`find -maxdepth 1 -type d -not -name "." -and -not -name ".svn" | sort`

for T in ${TESTS}; do
	if [ -e "${T}/info.txt" ]; then
		. ${T}/info.txt
		cd ${T}
		echo "run testcase: ${TITLE}"
		echo -n "result: "
		bash run.sh
		cd ..
	fi
done

