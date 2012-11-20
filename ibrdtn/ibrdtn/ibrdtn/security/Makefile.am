## sub directory

h_sources = \
	SecurityBlock.h \
	BundleAuthenticationBlock.h \
	PayloadIntegrityBlock.h \
	MutualSerializer.h \
	StrictSerializer.h \
	PayloadConfidentialBlock.h \
	ExtensionSecurityBlock.h \
	SecurityKey.h

cc_sources = \
	SecurityBlock.cpp \
	BundleAuthenticationBlock.cpp \
	PayloadIntegrityBlock.cpp \
	MutualSerializer.cpp \
	StrictSerializer.cpp \
	PayloadConfidentialBlock.cpp \
	ExtensionSecurityBlock.cpp \
	SecurityKey.cpp

#Install the headers in a versioned directory
library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)/security
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir) $(ibrcommon_CFLAGS) $(GLIB_CFLAGS)
AM_LDFLAGS = $(ibrcommon_LIBS) $(GLIB_LIBS)

if ANDROID
noinst_PROGRAMS = Android.mk
CLEANFILES = Android.mk
else
noinst_LTLIBRARIES= libsecurity.la
libsecurity_la_SOURCES= $(h_sources) $(cc_sources)
endif

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT $(GENERIC_LIBRARY_NAME) \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:STATIC libibrdtn_security \
		-:SOURCES $(h_sources) $(cc_sources) \
		-:CPPFLAGS $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) \
		-:LIBFILTER_STATIC ibrcommon \
		> $@