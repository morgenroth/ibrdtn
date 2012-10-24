#!/bin/bash
#

if [ -z "${1}" ]; then
  echo "Usage: ${0} <EID>"
  exit 1
fi

trim() { echo ${1}; }

HEXNAME="$(trim `echo -n ${1} | hexdump -e '8/1 "%02x"'`)"

echo "Generate key for ${1}"
echo "Filename: ${HEXNAME}.pem"

openssl genrsa -out ${HEXNAME}.pem 2048
openssl rsa -in ${HEXNAME}.pem -pubout -out ${HEXNAME}.pub
cat ${HEXNAME}.pub >> ${HEXNAME}.pem

