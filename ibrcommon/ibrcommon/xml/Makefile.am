## sub directory

h_sources = XMLStreamHandler.h XMLStreamReader.h XMLStreamWriter.h
cc_sources = XMLStreamReader.cpp XMLStreamWriter.cpp

#Install the headers in a versioned directory
library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)/xml
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir) $(LIBXML_CFLAGS)
AM_LDFLAGS = $(LIBXML_LIBS)

noinst_LTLIBRARIES= libibrcommonxml.la
libibrcommonxml_la_SOURCES= $(h_sources) $(cc_sources)