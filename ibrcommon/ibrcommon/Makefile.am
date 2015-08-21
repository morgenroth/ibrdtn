## Source directory

SUBDIRS = data link net thread

if OPENSSL
SUBDIRS += ssl
endif

if LIBXML
SUBDIRS += xml
endif

nodist_noinst_HEADERS = config.h

h_sources = \
	appstreambuf.h \
	Exceptions.h \
	refcnt_ptr.h \
	TimeMeasurement.h \
	MonotonicClock.h \
	Logger.h \
	TLSExceptions.h \
	Iterator.h

cc_sources = \
	appstreambuf.cpp \
	TimeMeasurement.cpp \
	MonotonicClock.cpp \
	Logger.cpp

if ! WIN32
h_sources += SyslogStream.h
cc_sources += SyslogStream.cpp
endif

library_includedir = $(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)
nodist_library_include_HEADERS = ibrcommon.h
library_include_HEADERS = $(h_sources)

AUTOMAKE_OPTIONS = nostdinc
AM_CPPFLAGS = -I$(top_srcdir) $(PTHREAD_CFLAGS) $(NL_CFLAGS) $(NL_ROUTE_CFLAGS) $(NL_GENL_CFLAGS) $(GCOV_CFLAGS)
AM_LDFLAGS = $(PTHREAD_LIBS) $(NL_LIBS) $(NL_ROUTE_LIBS) $(NL_GENL_LIBS) $(GCOV_LIBS)

ibrcommon_libs = \
	data/libibrcommon_data.la \
	link/libibrcommon_link.la \
	net/libibrcommon_net.la \
	thread/libibrcommon_thread.la

if OPENSSL
ibrcommon_libs += ssl/libibrcommon_ssl.la ssl/gcm/libibrcommon_gcm.la
endif

if LIBXML
ibrcommon_libs += xml/libibrcommon_xml.la
endif

if ANDROID
noinst_DATA = Android.mk
CLEANFILES = Android.mk
else
lib_LTLIBRARIES = libibrcommon.la
libibrcommon_la_SOURCES = $(h_sources) $(cc_sources)
libibrcommon_la_LIBADD = $(ibrcommon_libs)
libibrcommon_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
if WIN32
libibrcommon_la_LDFLAGS += -no-undefined
endif
endif

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT $(GENERIC_LIBRARY_NAME) \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:SHARED ibrcommon \
		-:SOURCES $(h_sources) $(cc_sources) \
		-:CPPFLAGS $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) \
			$(ibrcommon_libs) \
			-llog \
		-:LIBFILTER_WHOLE ibrcommon_data ibrcommon_link ibrcommon_net ibrcommon_thread ibrcommon_ssl ibrcommon_gcm ibrcommon_xml \
			nl-3 nl-genl-3 nl-route-3 \
		-:SUBDIR $(patsubst %,ibrcommon/%, $(SUBDIRS)) \
		> $@
