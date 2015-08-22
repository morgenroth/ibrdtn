## sub directory

h_sources = \
	Conditional.h \
	Queue.h Mutex.h \
	MutexLock.h \
	Semaphore.h \
	Thread.h \
	Timer.h \
	AtomicCounter.h \
	ThreadsafeState.h \
	ThreadsafeReference.h \
	SharedReference.h \
	RWMutex.h \
	RWLock.h \
	SignalHandler.h

cc_sources = \
	Conditional.cpp \
	Mutex.cpp \
	MutexLock.cpp \
	Semaphore.cpp \
	Thread.cpp \
	Timer.cpp \
	AtomicCounter.cpp \
	RWMutex.cpp \
	RWLock.cpp \
	SignalHandler.cpp

#Install the headers in a versioned directory
library_includedir = $(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)/thread
library_include_HEADERS = $(h_sources)

AUTOMAKE_OPTIONS = nostdinc
AM_CPPFLAGS = -I$(top_srcdir) $(PTHREAD_CFLAGS) $(GCOV_CFLAGS)
AM_LDFLAGS = $(PTHREAD_LIBS) $(GCOV_LIBS)

if ANDROID
noinst_DATA = Android.mk
CLEANFILES = Android.mk
else
noinst_LTLIBRARIES = libibrcommon_thread.la
libibrcommon_thread_la_SOURCES = $(h_sources) $(cc_sources)
endif

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT $(GENERIC_LIBRARY_NAME) \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:STATIC libibrcommon_thread \
		-:SOURCES $(h_sources) $(cc_sources) \
		-:CPPFLAGS $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) \
		> $@