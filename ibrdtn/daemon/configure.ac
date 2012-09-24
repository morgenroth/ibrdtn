dnl --------------------------------
dnl ibrdtn package configuration
dnl --------------------------------

define([PKG_VERSION], m4_normalize(m4_include([version.inc])))

AC_INIT(ibrdtnd, [PKG_VERSION], morgenro@ibr.cs.tu-bs.de)
AC_CONFIG_SRCDIR(src/Main.cpp)
AM_INIT_AUTOMAKE
LT_INIT
AM_PATH_CPPUNIT(1.9.6)

AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_MAINTAINER_MODE

# Test for new silent rules and enable only if they are available
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl -----------------------------------------------
dnl define SVN revision
dnl -----------------------------------------------
define([svnversion], esyscmd([sh -c "svnversion|tr -d '\n'"]))dnl
AC_DEFINE(SVN_REVISION, "svnversion", [SVN Revision])

dnl -----------------------------------------------
dnl Checks for programs.
dnl -----------------------------------------------

AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP

AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

AC_LANG_CPLUSPLUS

# Checks for libraries.
AC_CHECK_LIB([rt], [clock_gettime])
AC_CHECK_LIB([m], [pow])

# check for ibrdtn library
LOCAL_IBRDTN_VERSION=0.8
LOCAL_CHECK_IBRDTN
AC_SUBST(ibrdtn_CFLAGS)
AC_SUBST(ibrdtn_LIBS)

# Checks for header files.
AC_CHECK_HEADERS([syslog.h pwd.h sys/inotify.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([gethostname socket])

# Check for presence of pdfLaTeX
AC_CHECK_PROG(PDFLATEX, pdflatex, pdflatex)
if test -z "$PDFLATEX"; then
  AC_MSG_WARN([Unable to create PDF version of the documentation.])
fi

AM_CONDITIONAL([HAVE_PDFLATEX], [test x$PDFLATEX = xpdflatex])

PKG_CHECK_MODULES(DAEMON, libdaemon >= 0.12, [
	use_daemon="yes"
	AC_SUBST(DAEMON_CFLAGS)
	AC_SUBST(DAEMON_LIBS)
	AC_DEFINE(HAVE_LIBDAEMON, [1], ["daemon library is available"])
	AC_CHECK_LIB(daemon, daemon_reset_sigs, [
		AC_DEFINE(HAVE_DAEMON_RESET_SIGS, [1], ["daemon library has daemon_reset_sigs() and daemon_unblock_sigs() functions"])
	])
], [
	use_daemon="no"
	AC_MSG_WARN([daemon library not found, daemonize features disabled])
])

AC_ARG_WITH(curl,
	AS_HELP_STRING([--with-curl], [Build with libcurl
		@<:@default=no@:>@]),
	[
	if test "x$with_curl" = "xyes"; then
		PKG_CHECK_MODULES(CURL, libcurl >= 7.16.4, [
			use_curl="yes"
			AC_SUBST(CURL_CFLAGS)
			AC_SUBST(CURL_LIBS)
			AC_DEFINE(HAVE_LIBCURL, [1], ["curl library is available"])
		], [
			use_curl="no"
			AC_MSG_WARN([curl extension requested, but no curl library is found.])
		])
	else
		use_curl="no"
	fi
	],[
	use_curl="no"
])

AC_ARG_WITH(sqlite,
	AS_HELP_STRING([--with-sqlite], [Build with sqlite support
		@<:@default=no@:>@]),
	[
	if test "x$with_sqlite" = "xyes"; then
		PKG_CHECK_MODULES(SQLITE, sqlite3 >= 3.6, [
			use_sqlite="yes"
			AC_SUBST(SQLITE_CFLAGS)
			AC_SUBST(SQLITE_LIBS)
			AC_DEFINE(HAVE_SQLITE, [1], ["sqlite library is available"])
		], [
			use_sqlite="no"
			AC_MSG_WARN([sqlite extension requested, but no sqlite library is found.])
		])
	else
		use_sqlite="no"
	fi
	],[
	use_sqlite="no"
])

# Checks for lowpan support (libnl)
AC_ARG_WITH(lowpan,
	AS_HELP_STRING([--with-lowpan], [Build with lowpan support (support in ibrcommon needed)
		@<:@default=no@:>@]),
	[
	if test "x$with_lowpan" = "xyes"; then
		PKG_CHECK_MODULES([NL], [libnl-3.0], [
			use_lowpan="yes"
			AC_SUBST(NL_CFLAGS)
			AC_SUBST(NL_LIBS)
			AC_DEFINE(HAVE_LOWPAN_SUPPORT, [1], ["enable lowpan support"])
			AC_CHECK_HEADERS([arpa/inet.h])
			AC_CHECK_HEADERS([fcntl.h])
			AC_CHECK_HEADERS([limits.h])
			AC_CHECK_HEADERS([netinet/in.h])
			AC_CHECK_HEADERS([sys/socket.h])
		], [
			PKG_CHECK_MODULES([NL], [libnl-1], [
				use_lowpan="yes"
				AC_SUBST(NL_CFLAGS)
				AC_SUBST(NL_LIBS)
				AC_DEFINE(HAVE_LOWPAN_SUPPORT, [1], ["enable lowpan support"])
				AC_CHECK_HEADERS([arpa/inet.h])
				AC_CHECK_HEADERS([fcntl.h])
				AC_CHECK_HEADERS([limits.h])
				AC_CHECK_HEADERS([netinet/in.h])
				AC_CHECK_HEADERS([sys/socket.h])
			], [
				use_lowpan="no"
				AC_MSG_WARN([lowpan extension requested, but no libnl library is found. lowpan support is disabled.])
			])
		])
	else
		use_lowpan="no"
	fi
	],[
	use_lowpan="no"
])

dnl -----------------------------------------------
dnl optional parameter: compression support
dnl -----------------------------------------------

AC_ARG_WITH(compression,
	AS_HELP_STRING([--with-compression], [build with compression support
		@<:@default=no@:>@]),
	[
	if test "x$with_compression" = "xyes"; then
		AC_DEFINE(WITH_COMPRESSION, [1], ["bundle compression support enabled"])
	else
		with_compression="no"
	fi
	],[
	with_compression="no"
])

AM_CONDITIONAL(COMPRESSION, test x$with_compression = xyes)
AM_CONDITIONAL(CURL, test x$use_curl = xyes)
AM_CONDITIONAL(SQLITE, test x$use_sqlite = xyes)
AM_CONDITIONAL(LOWPAN, test x$use_lowpan = xyes)
AM_CONDITIONAL(DAEMON, test x$use_daemon = xyes)

dnl -----------------------------------------------
dnl optional parameter: DTN security support
dnl -----------------------------------------------

AC_ARG_WITH(dtnsec,
	AS_HELP_STRING([--with-dtnsec], [build with DTNSEC support
		@<:@default=no@:>@]),
	[
	if test "x$with_dtnsec" = "xyes"; then
		AC_DEFINE(WITH_BUNDLE_SECURITY, [1], ["bundle security support enabled"])
		CPPFLAGS="$CPPFLAGS -DWITH_BUNDLE_SECURITY"
	else
		with_dtnsec="no"
	fi
	],[
	with_dtnsec="no"
])

AM_CONDITIONAL(DTNSEC, test x$with_dtnsec = xyes)

dnl -----------------------------------------------
dnl optional parameter: TLS support
dnl -----------------------------------------------

AC_ARG_WITH(tls,
	AS_HELP_STRING([--with-tls], [build with TLS support
		@<:@default=no@:>@]),
	[
	if test "x$with_tls" = "xyes"; then
		AC_DEFINE(WITH_TLS, [1], ["tls support enabled"])
		CPPFLAGS="$CPPFLAGS -DWITH_TLS"
	else
		with_tls="no"
	fi
	],[
	with_tls="no"
])

AM_CONDITIONAL(TLS, test x$with_tls = xyes)

dnl -----------------------------------------------
dnl optional parameter: DHT support
dnl -----------------------------------------------

PKG_CHECK_MODULES([DTNDHT], dtndht >= 0.2.2 , [
			with_dht="yes"
			AC_SUBST(DTNDHT_CFLAGS)
			AC_SUBST(DTNDHT_LIBS)
			AC_DEFINE(WITH_DHT_NAMESERVICE, [1], ["dht nameservice support enabled"])
			AC_CHECK_HEADERS([arpa/inet.h])
			AC_CHECK_HEADERS([fcntl.h])
			AC_CHECK_HEADERS([netinet/in.h])
			AC_CHECK_HEADERS([sys/socket.h])
		], [
			with_dht="no"
			AC_MSG_WARN([dtndht library not found. dht support is disabled.])
		])
AM_CONDITIONAL(DTNDHT, test x$with_dht = xyes)

dnl -----------------------------------------------
dnl Generates Makefiles, configuration files and scripts
dnl -----------------------------------------------

AC_CONFIG_FILES([Makefile \
	src/Makefile \
	src/core/Makefile \
	src/net/Makefile \
	src/storage/Makefile \
	src/routing/Makefile \
	src/routing/epidemic/Makefile \
	src/routing/flooding/Makefile \
	src/routing/prophet/Makefile \
	src/security/Makefile \
	src/api/Makefile \
	etc/Makefile \
	doc/Makefile \
	man/Makefile \
	tests/Makefile \
	tests/unittests/Makefile])
	
AC_OUTPUT
