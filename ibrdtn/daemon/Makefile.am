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
	debian/ibrdtnd.logrotate \
	debian/ibrdtnd.default \
	debian/compat \
	debian/copyright \
	debian/changelog \
	debian/postinst \
	debian/rules \
	debian/init.d \
	debian/conffiles \
	debian/control \
	debian/docs \
	debian/source \
	debian/source/format

ACLOCAL_AMFLAGS = -I m4

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT dtnd \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:SUBDIR $(SUBDIRS) \
		> $@
