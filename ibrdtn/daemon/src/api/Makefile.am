## sub directory

AM_CPPFLAGS = -I$(top_srcdir)/src $(ibrdtn_CFLAGS) $(GCOV_CFLAGS)
AM_LDFLAGS = $(ibrdtn_LIBS) $(GCOV_LIBS)

api_SOURCES = \
	ApiServer.h \
	ApiServer.cpp \
	ClientHandler.cpp \
	ClientHandler.h \
	ExtendedApiHandler.cpp \
	ExtendedApiHandler.h \
	Registration.h \
	Registration.cpp \
	BinaryStreamClient.h \
	BinaryStreamClient.cpp \
	ManagementConnection.h \
	ManagementConnection.cpp \
	EventConnection.h \
	EventConnection.cpp \
	OrderedStreamHandler.h \
	OrderedStreamHandler.cpp \
	BundleStreamBuf.h \
	BundleStreamBuf.cpp \
	ApiP2PExtensionHandler.h \
	ApiP2PExtensionHandler.cpp \
	NativeSession.h \
	NativeSession.cpp \
	NativeSerializer.h \
	NativeSerializer.cpp \
	NativeSerializerCallback.h \
	NativeSerializerCallback.cpp

if ANDROID
noinst_DATA = Android.mk
CLEANFILES = Android.mk
else
noinst_LTLIBRARIES = libapi.la
libapi_la_SOURCES = $(api_SOURCES)
endif

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT dtnd \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:STATIC libdtnd_api \
		-:SOURCES $(api_SOURCES) \
		-:CPPFLAGS $(CPPFLAGS) $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) \
		> $@
