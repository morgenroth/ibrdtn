#!/bin/bash
#
#

if [ -z ${1} ]; then
	echo "please specify the mount directory as first parameter"
	exit 1;
fi

MOUNTDIR=${1}

while [ 1 == 1 ]; do
	inotifywait -e create -e delete ${MOUNTDIR}
	FILES=`find ${MOUNTDIR} -maxdepth 2 -type f -name '.dtndrive'`
	for DESC in $FILES; do
		DTNPATH=`dirname ${DESC}`
		echo "dtn drive directory found: ${DTNPATH}"

		# gen lock file
		LOCK="/tmp/`basename ${DTNPATH}.dtndrive`"

		# check if the path is already mounted
		if [ ! -e ${LOCK} ]; then
			cp ${DESC} ${LOCK}
			echo "DTNPATH=\"${DTNPATH}\"" >> ${LOCK}

			# load configuration
			. ${DESC}

			# announce node to the dtnd
			echo -e "protocol management\nconnection ${EID} file add file://${DTNPATH}/${STORAGE}" | netcat localhost 4550 > /dev/null
		fi
	done

	# check if some paths are gone
	LOCKS=`find /tmp -maxdepth 1 -type f -name '*.dtndrive'`
	for L in ${LOCKS}; do
		echo "check lock file: ${L}"
		. ${L}
		if [ ! -d ${DTNPATH} ]; then
			echo "medium removed"
			rm ${L}

			# announce node to the dtnd
			echo -e "protocol management\nconnection ${EID} file del file://${DTNPATH}/${STORAGE}" | netcat localhost 4550  > /dev/null
		fi
	done
done

