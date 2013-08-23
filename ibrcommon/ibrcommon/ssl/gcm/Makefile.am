h_sources = \
	brg_endian.h \
	brg_types.h \
	gcm_aes.h \
	gcm.h \
	gf128mul.h \
	mode_hdr.h

cc_sources = \
	gcm_aes.c \
	gcm.cpp \
	gf128mul.cpp

#Install the headers in a versioned directory
library_includedir = $(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)/ssl/gcm
library_include_HEADERS = $(h_sources)

AUTOMAKE_OPTIONS = nostdinc
AM_CPPFLAGS = -I$(top_srcdir)
AM_LDFLAGS = 

if ANDROID
noinst_DATA = Android.mk
CLEANFILES = Android.mk
else
noinst_LTLIBRARIES = libibrcommon_gcm.la
libibrcommon_gcm_la_SOURCES = $(h_sources) $(cc_sources)
endif

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT $(GENERIC_LIBRARY_NAME) \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:STATIC libibrcommon_gcm \
		-:SOURCES $(h_sources) $(cc_sources) \
		-:CPPFLAGS $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) \
		> $@