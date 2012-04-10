## Source directory
AUTOMAKE_OPTIONS = foreign

SUBDIRS = unittests

h_sources = 
cc_sources = 
				
# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = @ibrdtn_CFLAGS@ @CPPUNIT_CFLAGS@ -Wall
AM_LDFLAGS = @ibrdtn_LIBS@ @CPPUNIT_LIBS@

if CURL
AM_CPPFLAGS += @CURL_CFLAGS@
AM_LDFLAGS += @CURL_LIBS@
endif

if SQLITE
AM_CPPFLAGS += @SQLITE_CFLAGS@
AM_LDFLAGS += @SQLITE_LIBS@
endif

INCLUDES = -I@top_srcdir@

check_PROGRAMS = testsuite
testsuite_LDADD = @top_srcdir@/src/libdtnd.la
testsuite_SOURCES = $(h_sources) $(cc_sources) testsuite.cpp

TESTS = testsuite
