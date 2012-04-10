## sub directory

h_sources = Conditional.h Queue.h Mutex.h MutexLock.h Semaphore.h Thread.h Timer.h AtomicCounter.h ObjectLock.h ThreadsafeState.h ThreadsafeReference.h RWMutex.h RWLock.h
cc_sources = Conditional.cpp Mutex.cpp MutexLock.cpp Semaphore.cpp Thread.cpp Timer.cpp AtomicCounter.cpp ObjectLock.cpp RWMutex.cpp RWLock.cpp

#Install the headers in a versioned directory
library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)/thread
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir) $(PTHREAD_CFLAGS)
AM_LDFLAGS = $(PTHREAD_LIBS)

noinst_LTLIBRARIES= libthread.la
libthread_la_SOURCES= $(h_sources) $(cc_sources)
