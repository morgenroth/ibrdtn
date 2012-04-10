## sub directory
SUBDIRS = epidemic flooding prophet

AM_CPPFLAGS = -I$(top_srcdir) @ibrdtn_CFLAGS@
AM_LDFLAGS = @ibrdtn_LIBS@

routing_SOURCES = BaseRouter.cpp \
				BaseRouter.h \
				BundleSummary.cpp \
				BundleSummary.h \
				NeighborDatabase.cpp \
				NeighborDatabase.h \
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
				StaticRouteChangeEvent.cpp \
				StaticRouteChangeEvent.h \
				SummaryVector.cpp \
				SummaryVector.h \
				NodeHandshake.h \
				NodeHandshake.cpp \
				NodeHandshakeEvent.h \
				NodeHandshakeEvent.cpp \
				NodeHandshakeExtension.h \
				NodeHandshakeExtension.cpp
				
noinst_LTLIBRARIES = librouting.la
librouting_la_SOURCES= $(routing_SOURCES)
librouting_la_LIBADD = flooding/librtflooding.la epidemic/librtepidemic.la prophet/librtprophet.la
