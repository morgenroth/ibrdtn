
h_sources = brg_endian.h brg_types.h gcm_aes.h gcm.h gf128mul.h mode_hdr.h
cc_sources = gcm_aes.c gcm.cc gf128mul.cc

#Install the headers in a versioned directory
library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)/ssl/gcm
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir)
AM_LDFLAGS = 

noinst_LTLIBRARIES= libgcm.la
libgcm_la_SOURCES= $(h_sources) $(cc_sources)
