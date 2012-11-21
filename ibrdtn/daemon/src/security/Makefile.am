## sub directory

security_SOURCES =

if DTNSEC
security_SOURCES += \
	SecurityManager.h \
	SecurityManager.cpp \
	SecurityKeyManager.h \
	SecurityKeyManager.cpp
endif

if TLS
security_SOURCES += \
	TLSStreamComponent.h \
	TLSStreamComponent.cpp \
	SecurityCertificateManager.h \
	SecurityCertificateManager.cpp \
	CertificateManagerInitEvent.cpp
endif

AM_CPPFLAGS = -I$(top_srcdir)/src $(ibrdtn_CFLAGS)
AM_LDFLAGS = $(ibrdtn_LIBS)

if ANDROID
noinst_DATA = Android.mk
CLEANFILES = Android.mk
else
noinst_LTLIBRARIES = libsecurity.la
libsecurity_la_SOURCES= $(security_SOURCES)
endif

Android.mk: Makefile.am
	$(ANDROGENIZER) -:PROJECT dtnd \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:STATIC libdtnd_security \
		-:SOURCES $(security_SOURCES) \
		-:CPPFLAGS $(CPPFLAGS) $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) \
		-:LIBFILTER_STATIC ibrdtn \
		> $@
