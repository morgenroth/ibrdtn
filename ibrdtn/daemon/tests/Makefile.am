## Source directory
AUTOMAKE_OPTIONS = foreign

SUBDIRS = unittests

h_sources = tools/EventSwitchLoop.h
cc_sources = 

# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = -I$(top_srcdir)/src $(ibrdtn_CFLAGS) $(CPPUNIT_CFLAGS) -Wall $(CURL_CFLAGS) $(SQLITE_CFLAGS)
AM_LDFLAGS = $(ibrdtn_LIBS) $(CPPUNIT_LIBS) $(CURL_LIBS) $(SQLITE_LIBS)

check_PROGRAMS = testsuite
testsuite_LDADD = $(top_srcdir)/src/libdtnd.la
testsuite_SOURCES = $(h_sources) $(cc_sources) testsuite.cpp

TESTS = testsuite
