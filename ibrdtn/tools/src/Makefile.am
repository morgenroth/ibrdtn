AUTOMAKE_OPTIONS = foreign subdir-objects

# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = @ibrdtn_CFLAGS@ @DAEMON_CFLAGS@ @ARCHIVE_CFLAGS@  @tffs_CFLAGS@
AM_LDFLAGS = @ibrdtn_LIBS@ @DAEMON_LIBS@ @ARCHIVE_LIBS@ @tffs_LIBS@


INCLUDES = -I@top_srcdir@/src

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = dtnping dtninbox dtnoutbox dtnrecv dtnsend dtntracepath dtntrigger dtnconvert dtnstream

if TUNNEL
sbin_PROGRAMS = dtntunnel
endif

dtnping_SOURCES = dtnping.cpp
dtninbox_SOURCES = dtninbox.cpp \
		  io/TarUtils.h \
		  io/TarUtils.cpp
dtnoutbox_SOURCES = dtnoutbox.cpp \
		  tffs/tffs.h \
		  io/FATFile.h \
		  io/FATFile.cpp \
		  io/ObservedFile.h \
		  io/ObservedFile.cpp \
		  io/ObservedFATFile.h \
		  io/ObservedFATFile.cpp \
		  io/ObservedNormalFile.h \
		  io/ObservedNormalFile.cpp \
		  io/TarUtils.h \
		  io/TarUtils.cpp
dtnrecv_SOURCES = dtnrecv.cpp
dtnsend_SOURCES = dtnsend.cpp
dtntunnel_SOURCES = dtntunnel.cpp
dtntracepath_SOURCES = dtntracepath.cpp
dtntrigger_SOURCES = dtntrigger.cpp
dtnconvert_SOURCES = dtnconvert.cpp

dtnstream_SOURCES = \
	dtnstream.cpp \
	streaming/BundleStream.cpp \
	streaming/BundleStream.h \
	streaming/BundleStreamBuf.cpp \
	streaming/BundleStreamBuf.h \
	streaming/StreamBundle.cpp \
	streaming/StreamBundle.h
