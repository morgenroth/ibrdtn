## sub directory
SUBDIRS = epidemic flooding prophet

routing_SOURCES = \
	RoutingExtension.h \
	RoutingExtension.cpp \
	BaseRouter.cpp \
	BaseRouter.h \
	NeighborDatabase.cpp \
	NeighborDatabase.h \
	NeighborDataset.h \
	NeighborDataset.cpp \
	NeighborRoutingExtension.cpp \
	NeighborRoutingExtension.h \
	QueueBundleEvent.cpp \
	QueueBundleEvent.h \
	RequeueBundleEvent.cpp \
	RequeueBundleEvent.h \
	RetransmissionExtension.cpp \
	RetransmissionExtension.h \
	StaticRoutingExtension.cpp \
	StaticRoutingExtension.h \
	StaticRoute.h \
	StaticRoute.cpp \
	StaticRouteChangeEvent.cpp \
	StaticRouteChangeEvent.h \
	NodeHandshake.h \
	NodeHandshake.cpp \
	NodeHandshakeEvent.h \
	NodeHandshakeEvent.cpp \
	NodeHandshakeExtension.h \
	NodeHandshakeExtension.cpp \
	SchedulingBundleIndex.h \
	SchedulingBundleIndex.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src $(ibrdtn_CFLAGS) $(GCOV_CFLAGS)
AM_LDFLAGS = $(ibrdtn_LIBS) $(GCOV_LIBS)

librouting_la_LIBADD = flooding/librtflooding.la epidemic/librtepidemic.la prophet/librtprophet.la

if REGEX
routing_SOURCES += StaticRegexRoute.h StaticRegexRoute.cpp
endif

if ANDROID
noinst_DATA = Android.mk
CLEANFILES = Android.mk
else
noinst_LTLIBRARIES = librouting.la
librouting_la_SOURCES= $(routing_SOURCES)
endif

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT dtnd \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:STATIC libdtnd_routing \
		-:SOURCES $(routing_SOURCES) \
		-:CPPFLAGS $(CPPFLAGS) $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) \
			$(subst lib,libdtnd_, $(librouting_la_LIBADD)) \
		-:LIBFILTER_WHOLE dtnd_rtflooding dtnd_rtepidemic dtnd_rtprophet \
		-:SUBDIR $(patsubst %,src/routing/%, $(SUBDIRS)) \
		> $@
