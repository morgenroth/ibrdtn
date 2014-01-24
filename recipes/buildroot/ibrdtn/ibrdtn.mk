#############################################################
#
# ibrdtn
#
#############################################################
IBRDTN_VERSION:=0.12.0
IBRDTN_SOURCE:=ibrdtn-$(IBRDTN_VERSION).tar.gz
IBRDTN_SITE:=http://www.ibr.cs.tu-bs.de/projects/ibr-dtn/releases
IBRDTN_LIBTOOL_PATCH:=NO
IBRDTN_DEPENDENCIES:=host-pkgconf ibrcommon openssl zlib
IBRDTN_INSTALL_STAGING:=YES
IBRDTN_INSTALL_TARGET:=YES

IBRDTN_CONF_OPT =	\
		--with-dtnsec \
		--with-compression

$(eval $(autotools-package))
