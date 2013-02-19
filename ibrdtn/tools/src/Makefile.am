AUTOMAKE_OPTIONS = foreign

# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = @ibrdtn_CFLAGS@
AM_LDFLAGS = @ibrdtn_LIBS@

INCLUDES = -I@top_srcdir@/src

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = dtnping dtninbox dtnoutbox dtnrecv dtnsend dtntracepath dtntrigger dtnconvert dtnstream dtnrecv-ng

if TUNNEL
sbin_PROGRAMS = dtntunnel
endif

dtnping_SOURCES = dtnping.cpp
dtninbox_SOURCES = dtninbox.cpp
dtnoutbox_SOURCES = dtnoutbox.cpp
dtnrecv_ng_SOURCES = dtnrecv-ng.cpp
dtnrecv_SOURCES = dtnrecv.cpp
dtnsend_SOURCES = dtnsend.cpp
dtntunnel_SOURCES = dtntunnel.cpp
dtntracepath_SOURCES = dtntracepath.cpp
dtntrigger_SOURCES = dtntrigger.cpp
dtnconvert_SOURCES = dtnconvert.cpp
dtnstream_SOURCES = dtnstream.cpp BundleStream.h
