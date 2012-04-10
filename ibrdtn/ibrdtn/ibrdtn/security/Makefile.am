## sub directory

h_sources = SecurityBlock.h BundleAuthenticationBlock.h PayloadIntegrityBlock.h MutualSerializer.h StrictSerializer.h PayloadConfidentialBlock.h ExtensionSecurityBlock.h SecurityKey.h
cc_sources = SecurityBlock.cpp BundleAuthenticationBlock.cpp PayloadIntegrityBlock.cpp MutualSerializer.cpp StrictSerializer.cpp PayloadConfidentialBlock.cpp ExtensionSecurityBlock.cpp SecurityKey.cpp

#Install the headers in a versioned directory
library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)/security
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir) @ibrcommon_CFLAGS@ @GLIB_CFLAGS@
AM_LDFLAGS = @ibrcommon_LIBS@ @GLIB_LIBS@

noinst_LTLIBRARIES= libsecurity.la
libsecurity_la_SOURCES= $(h_sources) $(cc_sources)
