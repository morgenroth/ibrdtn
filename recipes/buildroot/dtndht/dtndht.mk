#############################################################
#
# dtndht
#
#############################################################
DTNDHT_VERSION:=0.2.3
DTNDHT_SOURCE:=dtndht-$(DTNDHT_VERSION).tar.gz
DTNDHT_SITE:=http://www.ibr.cs.tu-bs.de/projects/ibr-dtn/releases
DTNDHT_LIBTOOL_PATCH:=NO
DTNDHT_DEPENDENCIES:=host-pkgconf openssl
DTNDHT_INSTALL_STAGING:=YES
DTNDHT_INSTALL_TARGET:=YES

$(eval $(autotools-package))

