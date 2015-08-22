## sub directory

h_sources = \
	socket.h \
	socketstream.h \
	ieee802154.h \
	stopandwait.h \
	vsocket.h \
	vinterface.h \
	vaddress.h

cc_sources = \
	socket.cpp \
	socketstream.cpp \
	stopandwait.cpp \
	vsocket.cpp \
	vinterface.cpp \
	vaddress.cpp

if LOWPAN
h_sources += lowpansocket.h lowpanstream.h
cc_sources += lowpansocket.cpp lowpanstream.cpp
endif

#Install the headers in a versioned directory
library_includedir = $(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)/net
library_include_HEADERS = $(h_sources)

AUTOMAKE_OPTIONS = nostdinc
AM_CPPFLAGS = -I$(top_srcdir) $(PTHREAD_CFLAGS) $(NL_CFLAGS) $(NL_ROUTE_CFLAGS) $(NL_GENL_CFLAGS) $(GCOV_CFLAGS)
AM_LDFLAGS = $(PTHREAD_LIBS) $(NL_LIBS) $(NL_ROUTE_LIBS) $(NL_GENL_LIBS) $(GCOV_LIBS)

if ANDROID
noinst_DATA = Android.mk
CLEANFILES = Android.mk
else
noinst_LTLIBRARIES = libibrcommon_net.la
libibrcommon_net_la_SOURCES = $(h_sources) $(cc_sources)
endif

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT $(GENERIC_LIBRARY_NAME) \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:STATIC libibrcommon_net \
		-:SOURCES $(h_sources) $(cc_sources) \
		-:CPPFLAGS $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) \
		-:LIBFILTER_STATIC nl-3 nl-genl-3 nl-route-3 \
		> $@
