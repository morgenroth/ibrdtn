## Source directory

SUBDIRS = api data streams utils

if DTNSEC
  SUBDIRS += security
endif

noinst_HEADERS = config.h

h_sources = ibrdtn.h
cc_sources = 

library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir) @ibrcommon_CFLAGS@
AM_LDFLAGS = @ibrcommon_LIBS@

lib_LTLIBRARIES= libibrdtn.la
libibrdtn_la_SOURCES= $(h_sources) $(cc_sources)
libibrdtn_la_LIBADD= api/libapi.la data/libdata.la streams/libstreams.la utils/libutils.la
libibrdtn_la_LDFLAGS= -version-info $(GENERIC_LIBRARY_VERSION) -release $(GENERIC_RELEASE)

if DTNSEC
  libibrdtn_la_LIBADD += security/libsecurity.la
endif
