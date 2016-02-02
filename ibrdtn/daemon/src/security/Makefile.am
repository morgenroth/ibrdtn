## sub directory
SUBDIRS = exchange

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
	SecurityCertificateManager.h \
	SecurityCertificateManager.cpp
endif

AM_CPPFLAGS = -I$(top_srcdir)/src $(ibrdtn_CFLAGS) $(OPENSSL_CFLAGS) $(GCOV_CFLAGS)
AM_LDFLAGS = $(ibrdtn_LIBS) $(OPENSSL_LIBS) $(GCOV_LIBS)

libsecurity_la_LIBADD = exchange/libsecexchange.la

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
			$(subst lib,libdtnd_, $(libsecurity_la_LIBADD)) \
		-:LIBFILTER_WHOLE dtnd_secexchange \
		-:SUBDIR $(patsubst %,src/security/%, $(SUBDIRS)) \
		> $@
