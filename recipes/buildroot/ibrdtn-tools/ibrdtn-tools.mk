#############################################################
#
# ibrdtn-tools
#
#############################################################
#IBRDTN_TOOLS_VERSION:=0.10.0
#IBRDTN_TOOLS_SOURCE:=ibrdtn-tools-$(IBRDTN_TOOLS_VERSION).tar.gz
IBRDTN_TOOLS_SITE:=/home/goltzsch/workspace/ibrdtn-repo/ibrdtn/tools
IBRDTN_TOOLS_SITE_METHOD:=local
IBRDTN_TOOLS_LIBTOOL_PATCH:=NO
IBRDTN_TOOLS_DEPENDENCIES:=libtffs openssl libarchive ibrcommon ibrdtn
IBRDTN_TOOLS_INSTALL_STAGING:=YES
IBRDTN_TOOLS_INSTALL_TARGET:=YES
IBRDTN_TOOLS_CONF_OPT = --with-tffs=${STAGING_DIR}/usr/

IBRDTN_TOOLS_MAKE_OPT += LDFLAGS+="-all-static -ldl"

$(eval $(autotools-package))
