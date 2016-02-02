## sub directory

h_sources = \
	BLOB.h \
	ConfigFile.h \
	File.h \
	BloomFilter.h \
	iobuffer.h \
	Base64Stream.h \
	Base64Reader.h \
	Base64.h

cc_sources = \
	BLOB.cpp \
	ConfigFile.cpp \
	File.cpp \
	BloomFilter.cpp \
	iobuffer.cpp \
	Base64Stream.cpp \
	Base64Reader.cpp \
	Base64.cpp

#Install the headers in a versioned directory
library_includedir = $(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)/data
library_include_HEADERS = $(h_sources)

AUTOMAKE_OPTIONS = nostdinc
AM_CPPFLAGS = -I$(top_srcdir) $(PTHREAD_CFLAGS) $(GCOV_CFLAGS)
AM_LDFLAGS = $(PTHREAD_LIBS) $(GCOV_LIBS)

if ANDROID
noinst_DATA = Android.mk
CLEANFILES = Android.mk
else
noinst_LTLIBRARIES = libibrcommon_data.la
libibrcommon_data_la_SOURCES = $(h_sources) $(cc_sources)
endif

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT $(GENERIC_LIBRARY_NAME) \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:STATIC libibrcommon_data \
		-:SOURCES $(h_sources) $(cc_sources) \
		-:CPPFLAGS $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) \
		> $@
