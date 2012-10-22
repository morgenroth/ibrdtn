## top directory

#Build in these directories:
SUBDIRS = $(GENERIC_LIBRARY_NAME) tests

#Distribute these directories:
DIST_SUBDIRS = $(GENERIC_LIBRARY_NAME) tests

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(GENERIC_LIBRARY_NAME).pc

# debian files for package building
EXTRA_DIST= \
	debian/ibrdtn1.dirs \
	debian/compat \
	debian/ibrdtn1.install \
	debian/copyright \
	debian/changelog \
	debian/ibrdtn-dev.install \
	debian/rules \
	debian/ibrdtn-dev.dirs \
	debian/control \
	debian/docs \
	debian/source \
	debian/source/format

ACLOCAL_AMFLAGS = -I m4
