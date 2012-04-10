AUTOMAKE_OPTIONS = foreign
SUBDIRS = core storage net routing api
noinst_HEADERS = config.h

# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = @ibrdtn_CFLAGS@ 
AM_LDFLAGS = @ibrdtn_LIBS@

if CURL
AM_CPPFLAGS += @CURL_CFLAGS@
AM_LDFLAGS += @CURL_LIBS@
endif

if SQLITE
AM_CPPFLAGS += @SQLITE_CFLAGS@
AM_LDFLAGS += @SQLITE_LIBS@
endif

if DAEMON
AM_CPPFLAGS += @DAEMON_CFLAGS@
AM_LDFLAGS += @DAEMON_LIBS@
endif

if DTNDHT
AM_CPPFLAGS += @DTNDHT_CFLAGS@
AM_LDFLAGS += @DTNDHT_LIBS@
endif

if DTNSEC
SUBDIRS += security
else
if TLS
SUBDIRS += security
endif
endif

INCLUDES = -I@top_srcdir@/src

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
sbin_PROGRAMS = dtnd

daemon_SOURCES = \
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
				StandByManager.cpp


noinst_LTLIBRARIES = libdtnd.la
libdtnd_la_SOURCES= $(daemon_SOURCES)
libdtnd_la_LIBADD = core/libcore.la storage/libstorage.la net/libnet.la routing/librouting.la api/libapi.la

if DTNSEC
libdtnd_la_LIBADD += security/libsecurity.la
else
if TLS
libdtnd_la_LIBADD += security/libsecurity.la
endif
endif

dtnd_SOURCES = Main.cpp
dtnd_LDADD = libdtnd.la
