## sub directory

AM_CPPFLAGS = -I$(top_srcdir) @ibrdtn_CFLAGS@
AM_LDFLAGS = @ibrdtn_LIBS@

security_SOURCES =

if DTNSEC
security_SOURCES +=	\
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
				
noinst_LTLIBRARIES = libsecurity.la
libsecurity_la_SOURCES= $(security_SOURCES)
