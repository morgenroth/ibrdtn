AUTOMAKE_OPTIONS = foreign subdir-objects

# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = -I@top_srcdir@/src @ibrdtn_CFLAGS@ @DAEMON_CFLAGS@ @ARCHIVE_CFLAGS@  @tffs_CFLAGS@
AM_LDFLAGS = @ibrdtn_LIBS@ @DAEMON_LIBS@ @ARCHIVE_LIBS@ @tffs_LIBS@

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = dtnping dtnrecv dtnsend dtntracepath dtntrigger dtnconvert dtnstream

# compile dtninbox and dtnoutbox if libarchive is present
if LIBARCHIVE
bin_PROGRAMS += dtninbox dtnoutbox
endif

if TUNNEL
sbin_PROGRAMS = dtntunnel
endif

if LIBARCHIVE
tarutils_src = \
		io/FileHash.h \
		io/FileHash.cpp \
		io/ObservedFile.h \
		io/ObservedFile.cpp \
		io/TarUtils.h \
		io/TarUtils.cpp

if TFFS
tarutils_src += \
		io/FATFile.h \
		io/FATFile.cpp \
		io/FatImageReader.h \
		io/FatImageReader.cpp
endif

dtninbox_SOURCES = dtninbox.cpp $(tarutils_src)
dtnoutbox_SOURCES = dtnoutbox.cpp $(tarutils_src)
endif

dtnping_SOURCES = dtnping.cpp
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
