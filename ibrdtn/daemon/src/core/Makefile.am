## sub directory

AM_CPPFLAGS = -I$(top_srcdir)/src $(ibrdtn_CFLAGS)
AM_LDFLAGS = $(ibrdtn_LIBS)

core_SOURCES = \
	AbstractWorker.cpp \
	AbstractWorker.h \
	BundleCore.cpp \
	BundleCore.h \
	BundleEvent.cpp \
	BundleEvent.h \
	BundleExpiredEvent.cpp \
	BundleExpiredEvent.h \
	BundleGeneratedEvent.cpp \
	BundleGeneratedEvent.h \
	WallClock.cpp \
	WallClock.h \
	CustodyEvent.cpp \
	CustodyEvent.h \
	Event.cpp \
	EventDebugger.cpp \
	EventDebugger.h \
	Event.h \
	EventReceiver.cpp \
	EventReceiver.h \
	EventSwitch.cpp \
	EventSwitch.h \
	GlobalEvent.cpp \
	GlobalEvent.h \
	Node.cpp \
	NodeEvent.cpp \
	NodeEvent.h \
	Node.h \
	StatusReportGenerator.cpp \
	StatusReportGenerator.h \
	TimeEvent.cpp \
	TimeEvent.h \
	FragmentManager.h \
	FragmentManager.cpp \
	BundlePurgeEvent.h \
	BundlePurgeEvent.cpp \
	TimeAdjustmentEvent.h \
	TimeAdjustmentEvent.cpp

if ANDROID
noinst_PROGRAMS = Android.mk
CLEANFILES = Android.mk
else
noinst_LTLIBRARIES = libcore.la
libcore_la_SOURCES= $(core_SOURCES)
endif

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT dtnd \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:STATIC libdtnd_core \
		-:SOURCES $(core_SOURCES) \
		-:CPPFLAGS $(CPPFLAGS) $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) \
		-:LIBFILTER_STATIC ibrdtn \
		> $@
