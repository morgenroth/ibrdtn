AUTOMAKE_OPTIONS = foreign

SUBDIRS = core storage net routing api

if DTNSEC
SUBDIRS += security
else
if TLS
SUBDIRS += security
endif
endif

noinst_HEADERS = config.h

dtnd_sources = \
	Component.cpp \
	Component.h \
	config.h \
	Configuration.cpp \
	Configuration.h \
	Debugger.cpp \
	Debugger.h \
	DevNull.cpp \
	DevNull.h \
	EchoWorker.cpp \
	EchoWorker.h \
	Notifier.cpp \
	Notifier.h \
	StatisticLogger.h \
	StatisticLogger.cpp \
	CapsuleWorker.cpp \
	CapsuleWorker.h \
	DTNTPWorker.cpp \
	DTNTPWorker.h \
	StandByManager.h \
	StandByManager.cpp \
	ibrdtnd.h \
	ibrdtnd.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src $(ibrdtn_CFLAGS) $(CURL_CFLAGS) $(SQLITE_CFLAGS) $(DAEMON_CFLAGS) $(DTNDHT_CFLAGS)
AM_LDFLAGS = $(ibrdtn_LIBS) $(CURL_LIBS) $(SQLITE_LIBS) $(DAEMON_LIBS) $(DTNDHT_LIBS)

dtnd_libs = core/libcore.la storage/libstorage.la net/libnet.la routing/librouting.la api/libapi.la

if DTNSEC
dtnd_libs += security/libsecurity.la
else
if TLS
dtnd_libs += security/libsecurity.la
endif
endif

if ANDROID
noinst_PROGRAMS = Android.mk
CLEANFILES = Android.mk
else
# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
sbin_PROGRAMS = dtnd
noinst_LTLIBRARIES = libdtnd.la
libdtnd_la_SOURCES = $(dtnd_sources)
libdtnd_la_LIBADD = $(dtnd_libs)
dtnd_SOURCES = Main.cpp
dtnd_LDADD = libdtnd.la
endif

# include openssl used by ibrcommon and zlib
Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT dtnd \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:EXECUTABLE dtnd \
		-:SOURCES $(dtnd_sources) \
			Main.cpp \
		-:CPPFLAGS $(CPPFLAGS) $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) \
			$(subst lib,libdtnd_, $(dtnd_libs)) \
			-lssl -lcrypto \
			-lz \
		-:LIBFILTER_WHOLE dtnd_core dtnd_storage dtnd_net dtnd_routing dtnd_api \
			ibrdtn \
		-:SUBDIR $(patsubst %,src/%, $(SUBDIRS)) \
		> $@

