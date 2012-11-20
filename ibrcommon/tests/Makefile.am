## Source directory

SUBDIRS=unittests stress

h_sources = \
		link/netlinktest.h \
		thread/MutexTests.h \
		thread/ThreadTest.h \
		thread/TimerTest.h \
		thread/QueueTest.h \
		net/tcpstreamtest.h \
		net/tcpclienttest.h

cc_sources = \
		link/netlinktest.cpp \
		thread/MutexTests.cpp \
		thread/ThreadTest.cpp \
		thread/TimerTest.cpp \
		thread/QueueTest.cpp \
		net/tcpstreamtest.cpp \
		net/tcpclienttest.cpp

if OPENSSL
h_sources += ssl/HashStreamTest.h \
		ssl/CipherStreamTest.h
		
cc_sources += ssl/HashStreamTest.cpp \
		ssl/CipherStreamTest.cpp
endif

EXTRA_DIST = base64-dec.dat base64-enc.dat

AM_CPPFLAGS = $(DEBUG_CFLAGS)
AM_LDFLAGS = -L@top_srcdir@/ibrcommon/.libs -librcommon

AM_CPPFLAGS += -I@top_srcdir@ -I@top_srcdir@/tests

check_PROGRAMS = testsuite
testsuite_CXXFLAGS = ${AM_CPPFLAGS} ${CPPUNIT_CFLAGS} -Wall
testsuite_LDFLAGS = ${AM_LDFLAGS} ${CPPUNIT_LIBS}
testsuite_SOURCES = $(h_sources) $(cc_sources) Main.cpp

TESTS = testsuite
