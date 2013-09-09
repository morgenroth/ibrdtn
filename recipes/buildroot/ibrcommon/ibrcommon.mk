#############################################################
#
# ibrcommon
#
#############################################################
IBRCOMMON_VERSION:=0.10.0
IBRCOMMON_SOURCE:=ibrcommon-$(IBRCOMMON_VERSION).tar.gz
IBRCOMMON_SITE:=http://www.ibr.cs.tu-bs.de/projects/ibr-dtn/releases
IBRCOMMON_LIBTOOL_PATCH:=NO
IBRCOMMON_DEPENDENCIES:=host-pkg-config libnl openssl
IBRCOMMON_INSTALL_STAGING:=YES
IBRCOMMON_INSTALL_TARGET:=YES

IBRCOMMON_CONF_OPT =	\
		--with-openssl

$(eval $(call AUTOTARGETS,package,ibrcommon))
