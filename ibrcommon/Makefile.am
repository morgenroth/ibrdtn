## top directory

#Build in these directories:
SUBDIRS= $(GENERIC_LIBRARY_NAME) tests 

#Distribute these directories:
DIST_SUBDIRS = $(GENERIC_LIBRARY_NAME) tests

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ibrcommon.pc

# debian files for package building
EXTRA_DIST= \
	debian/compat \
	debian/ibrcommon1.install \
	debian/copyright \
	debian/changelog \
	debian/rules \
	debian/ibrcommon1.dirs \
	debian/control \
	debian/docs \
	debian/source \
	debian/source/format \
	debian/ibrcommon-dev.install \
	debian/ibrcommon-dev.dirs

#EXTRA_DIST=autogen.sh

ACLOCAL_AMFLAGS = -I m4
