#############################################################
#
# ibrdtnd
#
#############################################################
IBRDTND_VERSION:=0.10.0
IBRDTND_SOURCE:=ibrdtnd-$(IBRDTND_VERSION).tar.gz
IBRDTND_SITE:=http://www.ibr.cs.tu-bs.de/projects/ibr-dtn/releases
IBRDTND_LIBTOOL_PATCH:=NO
IBRDTND_DEPENDENCIES:=ibrcommon ibrdtn libdaemon openssl zlib sqlite
IBRDTND_INSTALL_STAGING:=YES
IBRDTND_INSTALL_TARGET:=YES

IBRDTND_MAKE_OPT += LDFLAGS+="-all-static -ldl"

IBRDTND_CONF_OPT =	\
		--with-sqlite \
		--with-dtnsec \
		--with-compression \
		--with-tls
		

$(eval $(call AUTOTARGETS,package,ibrdtnd))
