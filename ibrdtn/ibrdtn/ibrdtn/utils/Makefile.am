## sub directory

h_sources = \
	Utils.h \
	Clock.h \
	Random.h

cc_sources = \
	Utils.cpp \
	Clock.cpp \
	Random.cpp

#Install the headers in a versioned directory
library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)/utils
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir) $(ibrcommon_CFLAGS) $(GCOV_CFLAGS)
AM_LDFLAGS = $(ibrcommon_LIBS) $(GCOV_LIBS)

if ANDROID
noinst_DATA = Android.mk
CLEANFILES = Android.mk
else
noinst_LTLIBRARIES= libutils.la
libutils_la_SOURCES= $(h_sources) $(cc_sources)
endif

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT $(GENERIC_LIBRARY_NAME) \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:STATIC libibrdtn_utils \
		-:SOURCES $(h_sources) $(cc_sources) \
		-:CPPFLAGS $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) \
		> $@