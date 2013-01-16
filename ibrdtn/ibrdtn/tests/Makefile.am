## Source directory

h_sources = data/TestBundleList.h data/TestBundleSet.h data/TestDictionary.h data/TestSerializer.h net/TestStreamConnection.h api/TestPlainSerializer.h utils/TestUtils.h data/TestExtensionBlock.h data/TestTrackingBlock.h
cc_sources = data/TestBundleList.cpp data/TestBundleSet.cpp data/TestDictionary.cpp data/TestSerializer.cpp net/TestStreamConnection.cpp api/TestPlainSerializer.cpp utils/TestUtils.cpp data/TestExtensionBlock.cpp data/TestTrackingBlock.cpp Main.cpp

if DTNSEC
h_sources += security/TestSecurityBlock.h security/PayloadConfidentialBlockTest.h
cc_sources += security/TestSecurityBlock.cpp security/PayloadConfidentialBlockTest.cpp
endif

if COMPRESSION
h_sources += data/TestCompressedPayloadBlock.h
cc_sources += data/TestCompressedPayloadBlock.cpp
endif

AM_CPPFLAGS = $(PTHREAD_CFLAGS) $(ibrcommon_CFLAGS)
AM_LDFLAGS = $(PTHREAD_LIBS) $(ibrcommon_LIBS) -L@top_srcdir@/ibrdtn/.libs -librcommon -librdtn

AM_CPPFLAGS += -I@top_srcdir@ -I@top_srcdir@/tests

check_PROGRAMS = testsuite
testsuite_CXXFLAGS = ${AM_CPPFLAGS} ${CPPUNIT_CFLAGS} -I../../src -Wall
testsuite_LDFLAGS = ${AM_LDFLAGS} ${CPPUNIT_LIBS}
testsuite_SOURCES = $(h_sources) $(cc_sources)

TESTS = testsuite
