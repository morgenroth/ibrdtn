## sub directory

h_sources = Serializer.h AgeBlock.h ScopeControlHopLimitBlock.h Block.h Bundle.h BundleID.h BundleList.h BundleMerger.h BundleString.h CustodySignalBlock.h Dictionary.h DTNTime.h EID.h Exceptions.h ExtensionBlock.h MetaBundle.h PayloadBlock.h PrimaryBlock.h SDNV.h StatusReportBlock.h BundleFragment.h StreamBlock.h
cc_sources = Serializer.cpp AgeBlock.cpp ScopeControlHopLimitBlock.cpp Block.cpp Bundle.cpp BundleID.cpp BundleList.cpp BundleMerger.cpp BundleString.cpp CustodySignalBlock.cpp Dictionary.cpp DTNTime.cpp EID.cpp ExtensionBlock.cpp MetaBundle.cpp PayloadBlock.cpp PrimaryBlock.cpp SDNV.cpp StatusReportBlock.cpp BundleFragment.cpp StreamBlock.cpp

if COMPRESSION
  h_sources += CompressedPayloadBlock.h
  cc_sources += CompressedPayloadBlock.cpp
endif

#Install the headers in a versioned directory
library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)/data
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir) @ibrcommon_CFLAGS@
AM_LDFLAGS = @ibrcommon_LIBS@

noinst_LTLIBRARIES= libdata.la
libdata_la_SOURCES= $(h_sources) $(cc_sources)
