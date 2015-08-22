## Source directory

SUBDIRS = api data streams utils

if DTNSEC
SUBDIRS += security
endif

nodist_noinst_HEADERS = config.h

h_sources =
cc_sources = dummy.cpp

library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)
nodist_library_include_HEADERS = ibrdtn.h
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir) $(ibrcommon_CFLAGS) $(GCOV_CFLAGS)
AM_LDFLAGS = $(ibrcommon_LIBS) $(GCOV_LIBS)

ibrdtn_libs = api/libapi.la data/libdata.la streams/libstreams.la utils/libutils.la

if DTNSEC
ibrdtn_libs += security/libsecurity.la
endif

if ANDROID
noinst_DATA = Android.mk
CLEANFILES = Android.mk
else
lib_LTLIBRARIES= libibrdtn.la
libibrdtn_la_SOURCES= $(h_sources) $(cc_sources)
libibrdtn_la_LIBADD= $(ibrdtn_libs)
libibrdtn_la_LDFLAGS= -version-info $(GENERIC_LIBRARY_VERSION)
if WIN32
libibrdtn_la_LDFLAGS += -no-undefined
endif
endif

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT $(GENERIC_LIBRARY_NAME) \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:SHARED ibrdtn \
		-:SOURCES $(h_sources) $(cc_sources) \
		-:CPPFLAGS $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) \
			$(subst lib,libibrdtn_, $(ibrdtn_libs)) \
			-lz \
			-lgnustl_shared \
		-:LIBFILTER_WHOLE ibrdtn_api ibrdtn_data ibrdtn_streams ibrdtn_utils ibrdtn_security \
		-:SUBDIR $(patsubst %,ibrdtn/%, $(SUBDIRS)) \
		> $@
