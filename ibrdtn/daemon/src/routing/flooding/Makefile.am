## sub directory

AM_CPPFLAGS = -I$(top_srcdir) @ibrdtn_CFLAGS@
AM_LDFLAGS = @ibrdtn_LIBS@

routing_SOURCES = FloodRoutingExtension.cpp \
			FloodRoutingExtension.h
			
noinst_LTLIBRARIES = librtflooding.la
librtflooding_la_SOURCES= $(routing_SOURCES)
