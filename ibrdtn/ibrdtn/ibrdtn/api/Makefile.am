## sub directory

h_sources = \
	BLOBBundle.h \
	APIClient.h \
	Client.h \
	FileBundle.h \
	StringBundle.h \
	Bundle.h \
	dtn_api.h \
	PlainSerializer.h

cc_sources = \
	BLOBBundle.cpp \
	FileBundle.cpp \
	StringBundle.cpp \
	Bundle.cpp \
	APIClient.cpp \
	Client.cpp \
	PlainSerializer.cpp

#Install the headers in a versioned directory
library_includedir = $(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)/api
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir) $(ibrcommon_CFLAGS)
AM_LDFLAGS = $(ibrcommon_LIBS)

if ANDROID
noinst_DATA = Android.mk
CLEANFILES = Android.mk
else
noinst_LTLIBRARIES = libapi.la
libapi_la_SOURCES = $(h_sources) $(cc_sources)
endif

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT $(GENERIC_LIBRARY_NAME) \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:STATIC libibrdtn_api \
		-:SOURCES $(h_sources) $(cc_sources) \
		-:CPPFLAGS $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) \
		-:LIBFILTER_STATIC ibrcommon \
		> $@