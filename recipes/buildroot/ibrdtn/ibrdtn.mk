#############################################################
#
# ibrdtn
#
#############################################################
IBRDTN_VERSION:=1.0.1
IBRDTN_SOURCE:=ibrdtn-$(IBRDTN_VERSION).tar.gz
IBRDTN_SITE:=http://www.ibr.cs.tu-bs.de/projects/ibr-dtn/releases
IBRDTN_LIBTOOL_PATCH:=NO
IBRDTN_DEPENDENCIES:=host-pkgconf ibrcommon zlib
IBRDTN_INSTALL_STAGING:=YES
IBRDTN_INSTALL_TARGET:=YES

IBRDTN_CONF_OPT =	\
		--with-compression

$(eval $(autotools-package))
