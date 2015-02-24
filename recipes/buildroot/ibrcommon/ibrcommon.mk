#############################################################
#
# ibrcommon
#
#############################################################
IBRCOMMON_VERSION:=1.0.1
IBRCOMMON_SOURCE:=ibrcommon-$(IBRCOMMON_VERSION).tar.gz
IBRCOMMON_SITE:=http://www.ibr.cs.tu-bs.de/projects/ibr-dtn/releases
IBRCOMMON_LIBTOOL_PATCH:=NO
IBRCOMMON_DEPENDENCIES:=host-pkgconf libnl openssl
IBRCOMMON_INSTALL_STAGING:=YES
IBRCOMMON_INSTALL_TARGET:=YES

IBRCOMMON_CONF_OPT =	\
		--with-openssl

ifeq ($(BR2_PTHREADS_OLD),y)
  IBRCOMMON_CONF_OPT += --disable-netlink
endif

$(eval $(autotools-package))
