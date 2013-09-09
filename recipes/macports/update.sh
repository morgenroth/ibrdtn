#!/bin/bash -xe
# generate macports files based on a packages URL

if [ $# -lt 1 ]; then
  echo "Usage: $0 <URL-to-packages-file>"
  exit 1
fi

# packages URL should be the first parameter
PACKAGES_URL="${1}"

# tools
WGET=`which wget`

escape_string() {
    echo "${1}" | sed 's/\//\\\//g'
}

${WGET} -O - ${PACKAGES_URL} | while read line
do
        PKG_NAME=`echo $line | awk '{print $1}'`
        PKG_VERSION=`echo $line | awk '{print $2}'`
        PKG_ARCHIVE=`echo $line | awk '{print $3}'`
        PKG_URL=`echo $line | awk '{print $4}'`

	PKG_MAINTAINER="schildt@ibr.cs.tu-bs.de"
        PKG_URL_ESCAPED=$(escape_string ${PKG_URL})

        MP_FOLDER=`find recipes/ -type d -name ${PKG_NAME}`
        MP_PORTFILE="${MP_FOLDER}/Portfile"

	# copy template to gen folder
	mkdir -p gen/${MP_FOLDER}
	cp -rv ${MP_FOLDER}/* gen/${MP_FOLDER}
	MP_PORTFILE_DEST="gen/${MP_FOLDER}/Portfile"

        # download archive
        mkdir -p dl
        ${WGET} -O dl/${PKG_ARCHIVE} ${PKG_URL}/${PKG_ARCHIVE}

        # create sha1 / md5 hash
	SHA1SUM=$(sha1sum dl/${PKG_ARCHIVE} | awk '{print $1}')
        MD5SUM=$(md5sum dl/${PKG_ARCHIVE} | awk '{print $1}')

	PKG_CHECKSUMS="${PKG_ARCHIVE} md5 ${MD5SUM} sha1 ${SHA1SUM}"

        # prepare patch file for ibrdtnd configuration
        #if [ "${PKG_NAME}" = "ibrdtnd" ]; then
            #PATCH_FILE=$(find gen/recipes/ -name ibrdtnd.conf.patch)

            #tar xvzf dl/${PKG_ARCHIVE}

            #DTND_CONFIG="${PKG_NAME}-${PKG_VERSION}/etc/ibrdtnd.conf"
            #mv ${DTND_CONFIG} ${DTND_CONFIG}.orig

            #cat ${DTND_CONFIG}.orig | sed 's/eth0/en0/' | \
            #    sed 's/#api_interface = any/api_interface = lo0/' | \
            #    sed 's/^user = dtnd/#user = dtnd/' > ${DTND_CONFIG}

            #echo "diff -rupN ${DTND_CONFIG}.orig ${DTND_CONFIG}; exit 0;" | bash > ${PATCH_FILE}

            # add hash sums for diff file
            #SHA1SUM=$(sha1sum ${PATCH_FILE} | awk '{print $1}')
            #MD5SUM=$(md5sum ${PATCH_FILE} | awk '{print $1}')
            #PKG_CHECKSUMS="${PKG_CHECKSUMS} ibrdtnd.conf.patch md5 ${MD5SUM} sha1 ${SHA1SUM}"

            # remove folder
            #rm -r ${PKG_NAME}-${PKG_VERSION}
        #fi

        PKG_VERSION_KEYWORD='${PKG_VERSION}'
        PKG_MAINTAINER_KEYWORD='${PKG_MAINTAINER}'
        PKG_CHECKSUMS_KEYWORD='${PKG_CHECKSUMS}'
        PKG_URL_KEYWORD='${PKG_URL}'

        # insert variables
	cat ${MP_PORTFILE} | \
		sed "s/${PKG_VERSION_KEYWORD}/${PKG_VERSION}/" | \
		sed "s/${PKG_MAINTAINER_KEYWORD}/${PKG_MAINTAINER}/" | \
		sed "s/${PKG_URL_KEYWORD}/${PKG_URL_ESCAPED}/" | \
		sed "s/${PKG_CHECKSUMS_KEYWORD}/${PKG_CHECKSUMS}/" > ${MP_PORTFILE_DEST}
done

