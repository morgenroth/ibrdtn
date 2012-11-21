## top directory

#Build in these directories:
SUBDIRS = $(GENERIC_LIBRARY_NAME)

if ANDROID
noinst_DATA = Android.mk
CLEANFILES = Android.mk
else
SUBDIRS += tests
endif

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

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT $(GENERIC_LIBRARY_NAME) \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:SUBDIR $(SUBDIRS) \
		> $@
