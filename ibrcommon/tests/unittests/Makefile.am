
noinst_HEADERS = \
	FileTest.hh \
	BLOBTest.hh \
	stopandwaitTest.hh \
	BloomFilterTest.hh \
	iobufferTest.h \
	Base64StreamTest.h \
	refcnt_ptrTest.hh
#	SyslogStreamTest.hh \
#	TimeMeasurementTest.hh \
#	SemaphoreTest.hh \
#	NetAddressTest.hh \
#	MulticastSocketTest.hh \
#	QueueTest.hh \
#	tcpstreamTest.hh \
#	LoggerTest.hh \
#	tcpclientTest.hh \
#	tcpserverTest.hh \
#	ConfigFileTest.hh \
#	ConditionalTest.hh \
#	udpsocketTest.hh \
#	UnicastSocketTest.hh \
#	AtomicCounterTest.hh \
#	ThreadTest.hh \
#	TimerTest.hh \
#	UnicastSocketLowpanTest.hh \
#	NetInterfaceTest.hh \
#	BroadcastSocketTest.hh \
#	lowpansocketTest.hh

unittest_SOURCES = \
	Main.cpp \
	FileTest.cpp \
	BLOBTest.cpp \
	stopandwaitTest.cpp \
	BloomFilterTest.cpp \
	iobufferTest.cpp \
	Base64StreamTest.cpp \
	refcnt_ptrTest.cpp
#	SyslogStreamTest.cpp \
#	TimeMeasurementTest.cpp \
#	SemaphoreTest.cpp \
#	NetAddressTest.cpp \
#	MulticastSocketTest.cpp \
#	QueueTest.cpp \
#	tcpstreamTest.cpp \
#	LoggerTest.cpp \
#	tcpclientTest.cpp \
#	tcpserverTest.cpp \
#	ConfigFileTest.cpp \
#	ConditionalTest.cpp \
#	udpsocketTest.cpp \
#	UnicastSocketTest.cpp \
#	AtomicCounterTest.cpp \
#	ThreadTest.cpp \
#	TimerTest.cpp \
#	UnicastSocketLowpanTest.cpp \
#	NetInterfaceTest.cpp \
#	BroadcastSocketTest.cpp \
#	lowpansocketTest.cpp

AM_CPPFLAGS = $(DEBUG_CFLAGS)
AM_LDFLAGS = -L@top_srcdir@/ibrcommon/.libs -librcommon

INCLUDES = -I@top_srcdir@ -I@top_srcdir@/tests/unittests

check_PROGRAMS = unittest
unittest_CXXFLAGS = ${AM_CPPFLAGS} ${CPPUNIT_CFLAGS} -Wall
unittest_LDFLAGS = ${AM_LDFLAGS} ${CPPUNIT_LIBS}

TESTS = unittest
