## sub directory

AM_CPPFLAGS = -I$(top_srcdir) @ibrdtn_CFLAGS@
AM_LDFLAGS = @ibrdtn_LIBS@

routing_SOURCES = ProphetRoutingExtension.cpp \
				ProphetRoutingExtension.h
				
noinst_LTLIBRARIES = librtprophet.la
librtprophet_la_SOURCES= $(routing_SOURCES)
