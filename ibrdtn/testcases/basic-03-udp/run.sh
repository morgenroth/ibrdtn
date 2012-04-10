#!/bin/bash
#

# run the daemon
dtnd -c ibrdtn01.conf --nodiscovery >/dev/null 2>&1 &
DTND1_PID=$!

dtnd -c ibrdtn02.conf --noapi --nodiscovery >/dev/null 2>&1 &
DTND2_PID=$!

sleep 5

PING_OUTPUT=`dtnping --count 1 dtn://node02.dtn/echo`
CHECK_TRANSMISSION=`echo ${PING_OUTPUT} | grep '1 bundles transmitted, 1 received'`

sleep 1
kill ${DTND1_PID} >/dev/null 2>&1
kill ${DTND2_PID} >/dev/null 2>&1

if [ -z "${CHECK_TRANSMISSION}" ]; then
	echo "error: dtnping failed"
	exit 1
fi

echo "success"
exit 0

