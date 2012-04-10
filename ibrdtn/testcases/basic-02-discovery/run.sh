#!/bin/bash
#

# cleanup
rm /tmp/dtnd-discovery.log >/dev/null 2>&1

# run the daemon
dtnd -c ibrdtn.conf --noapi >/tmp/dtnd-discovery.log 2>&1 &

# save the pid for later kill
DTND_PID=$!

sleep 1

# send discovery message to the daemon
cat disco-single.dat | netcat -q 10 -u 127.0.0.1 4551

sleep 1
kill ${DTND_PID} >/dev/null 2>&1


# check if the daemon log contains a discovery message
CHECK_AVAILABLE=`cat /tmp/dtnd-discovery.log | grep "dtn://vanda.ibr.cs.tu-bs.de" | grep " available"`
CHECK_UNAVAILABLE=`cat /tmp/dtnd-discovery.log | grep "dtn://vanda.ibr.cs.tu-bs.de" | grep " unavailable"`

if [ -z "${CHECK_AVAILABLE}" ]; then
	echo "error: no discovery message found"
	exit 1
fi

if [ -z "${CHECK_UNAVAILABLE}" ]; then
	echo "error: discovery does not timed out"
	exit 1
fi

echo "success"
exit 0
