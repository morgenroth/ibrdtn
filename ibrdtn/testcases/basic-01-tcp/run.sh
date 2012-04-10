#!/bin/bash
#

# cleanup
rm /tmp/dtnd-tcp-cl.dat

# run the daemon
dtnd -c ibrdtn.conf --noapi --nodiscovery >/dev/null 2>&1 &

# save the pid for later kill
DTND_PID=$!
#echo $! > /tmp/dtnd.pid

sleep 1

# connect with netcat on non-standard port
netcat localhost 4559 > /tmp/dtnd-tcp-cl.dat &
NC_PID=$!

sleep 1
kill ${NC_PID}  >/dev/null 2>&1
sleep 1
kill ${DTND_PID} >/dev/null 2>&1

MD5=`cat /tmp/dtnd-tcp-cl.dat | md5sum | sed 's/\(.*\)  -/\1/' | tr -d '\n'`

if [ "${MD5}" == "d38cff53168360301810ca86b3607cd0" ]; then
	echo "success"
	exit 0
else
	echo "failed"
	exit 1
fi
