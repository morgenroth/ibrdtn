noinst_HEADERS = StressModule.h StressBLOB.h
stresstest_SOURCES = StressBLOB.cpp Main.cpp

AM_CPPFLAGS = $(DEBUG_CFLAGS)
AM_LDFLAGS = -L@top_srcdir@/ibrcommon/.libs -librcommon

AM_CPPFLAGS += -I@top_srcdir@ -I@top_srcdir@/tests/stress

check_PROGRAMS = stresstest
stresstest_CXXFLAGS = ${AM_CPPFLAGS} ${CPPUNIT_CFLAGS} -Wall
stresstest_LDFLAGS = ${AM_LDFLAGS} ${CPPUNIT_LIBS}

