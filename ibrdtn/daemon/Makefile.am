AUTOMAKE_OPTIONS = foreign

SUBDIRS = src

if ANDROID
noinst_DATA = Android.mk
CLEANFILES = Android.mk
else
SUBDIRS += etc doc man scripts tests munin
endif

# debian files for package building
EXTRA_DIST= \
	debian/changelog \
	debian/compat \
	debian/control \
	debian/copyright \
	debian/ibrdtnd.default \
	debian/ibrdtnd.dirs \
	debian/ibrdtnd.docs \
	debian/ibrdtnd.init \
	debian/ibrdtnd.install \
	debian/ibrdtnd.logrotate \
	debian/ibrdtnd.postinst \
	debian/ibrdtnd.postrm \
	debian/ibrdtnd.preinst \
	debian/ibrdtnd.prerm \
	debian/munin-plugins-ibrdtnd.install \
	debian/rules \
	debian/source \
	debian/source/format

ACLOCAL_AMFLAGS = -I m4

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT dtnd \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:SUBDIR $(SUBDIRS) \
		> $@
