## sub directory

h_sources = StreamConnection.h StreamContactHeader.h StreamDataSegment.h
cc_sources = StreamBuffer.cpp StreamConnection.cpp StreamContactHeader.cpp StreamDataSegment.cpp

#Install the headers in a versioned directory
library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)/streams
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir) @ibrcommon_CFLAGS@
AM_LDFLAGS = @ibrcommon_LIBS@

noinst_LTLIBRARIES= libstreams.la
libstreams_la_SOURCES= $(h_sources) $(cc_sources)