## sub directory

routing_SOURCES = \
	EpidemicRoutingExtension.cpp \
	EpidemicRoutingExtension.h

AM_CPPFLAGS = -I$(top_srcdir)/src $(ibrdtn_CFLAGS)
AM_LDFLAGS = $(ibrdtn_LIBS)

if ANDROID
noinst_DATA = Android.mk
CLEANFILES = Android.mk
else
noinst_LTLIBRARIES = librtepidemic.la
librtepidemic_la_SOURCES= $(routing_SOURCES)
endif

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT dtnd \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:STATIC libdtnd_rtepidemic \
		-:SOURCES $(routing_SOURCES) \
		-:CPPFLAGS $(CPPFLAGS) $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) \
		> $@
