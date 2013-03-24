## Source directory

SUBDIRS = data link net thread

if OPENSSL
SUBDIRS += ssl
endif

if LIBXML
SUBDIRS += xml
endif

noinst_HEADERS = config.h

h_sources = \
	ibrcommon.h \
	appstreambuf.h \
	Exceptions.h \
	refcnt_ptr.h \
	SyslogStream.h \
	TimeMeasurement.h \
	Logger.h \
	TLSExceptions.h \
	Iterator.h

cc_sources = \
	appstreambuf.cpp \
	SyslogStream.cpp \
	TimeMeasurement.cpp \
	Logger.cpp

library_includedir = $(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir) $(PTHREAD_CFLAGS) $(NL_CFLAGS) $(NL_ROUTE_CFLAGS) $(NL_GENL_CFLAGS)
AM_LDFLAGS = $(PTHREAD_LIBS) $(NL_LIBS) $(NL_ROUTE_LIBS) $(NL_GENL_LIBS)

ibrcommon_libs = data/libdata.la link/liblink.la net/libnet.la thread/libthread.la

if OPENSSL
ibrcommon_libs += ssl/libssl.la ssl/gcm/libgcm.la
endif

if LIBXML
ibrcommon_libs += xml/libxml.la
endif

if ANDROID
noinst_DATA = Android.mk
CLEANFILES = Android.mk
else
lib_LTLIBRARIES = libibrcommon.la
libibrcommon_la_SOURCES = $(h_sources) $(cc_sources)
libibrcommon_la_LIBADD = $(ibrcommon_libs)
libibrcommon_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION) -release $(GENERIC_RELEASE)
endif

# include libnl as static lib
Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT $(GENERIC_LIBRARY_NAME) \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:STATIC ibrcommon \
		-:SOURCES $(h_sources) $(cc_sources) \
		-:CPPFLAGS $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) \
			$(subst lib,libibrcommon_, $(ibrcommon_libs)) \
		-:LIBFILTER_WHOLE ibrcommon_data ibrcommon_link ibrcommon_net ibrcommon_thread ibrcommon_ssl ibrcommon_gcm ibrcommon_xml \
			nl-3 nl-genl-3 nl-route-3 \
		-:SUBDIR $(patsubst %,ibrcommon/%, $(SUBDIRS)) \
		> $@
