## sub directory

h_sources = BLOB.h ConfigFile.h File.h BloomFilter.h iobuffer.h Base64Stream.h Base64Reader.h Base64.h
cc_sources = BLOB.cpp ConfigFile.cpp File.cpp BloomFilter.cpp iobuffer.cpp Base64Stream.cpp Base64Reader.cpp Base64.cpp

#Install the headers in a versioned directory
library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)/data
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir) $(PTHREAD_CFLAGS)
AM_LDFLAGS = $(PTHREAD_LIBS)

noinst_LTLIBRARIES= libdata.la
libdata_la_SOURCES= $(h_sources) $(cc_sources)