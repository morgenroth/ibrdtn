dnl --------------------------------
dnl Package name and version number
dnl --------------------------------

# include version information
m4_include([version.m4])

dnl --------------------------------
dnl Initialization macros.
dnl --------------------------------

AC_INIT([ibrcommon], PKG_FULL_VERSION, morgenro@ibr.cs.tu-bs.de)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(ibrcommon/ibrcommon.h)
AC_CONFIG_HEADER(ibrcommon/config.h)
LT_INIT
AM_PATH_CPPUNIT(1.9.6)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

# Test for new silent rules and enable only if they are available
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl --------------------------------
dnl library version number
dnl --------------------------------

#API version
GENERIC_API_VERSION=PKG_MAJOR_VERSION.PKG_MINOR_VERSION

#shared library versioning
GENERIC_LIBRARY_VERSION=PKG_LIB_VERSION
#                       | | |
#                +------+ | +---+
#                |        |     |
#             current:revision:age
#                |        |     |
#                |        |     +- increment if interfaces have been added
#                |        |        set to zero if interfaces have been removed
#                                  or changed
#                |        +- increment if source code has changed
#                |           set to zero if current is incremented
#                +- increment if interfaces have been added, removed or changed

# define the library name
GENERIC_LIBRARY_NAME=$PACKAGE_NAME
GENERIC_VERSION=$PACKAGE_VERSION
GENERIC_RELEASE=PKG_MAJOR_VERSION.PKG_MINOR_VERSION

AC_SUBST(GENERIC_API_VERSION)
AC_SUBST(GENERIC_LIBRARY_VERSION)
AC_SUBST(GENERIC_LIBRARY_NAME)
AC_SUBST(GENERIC_RELEASE)
AC_SUBST(GENERIC_VERSION)

dnl -----------------------------------------------
dnl Checks for programs.
dnl -----------------------------------------------

AC_PROG_CXX
AC_PROG_CC

AC_PROG_INSTALL
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AM_SANITY_CHECK

AC_LANG_CPLUSPLUS

AC_CHECK_LIB(rt, clock_gettime)

# Checks for header files.
AC_CHECK_FUNCS([bzero])
AC_CHECK_FUNCS([clock_gettime])
AC_CHECK_FUNCS([gethostname])
AC_CHECK_FUNCS([gettimeofday])
AC_CHECK_FUNCS([inet_ntoa])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([mkdir])
AC_CHECK_FUNCS([pow])
AC_CHECK_FUNCS([rmdir])
AC_CHECK_FUNCS([socket])
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([netdb.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([syslog.h])

# differ between Mac OSX and Linux
AC_CHECK_HEADERS([features.h mach/mach_time.h sys/semaphore.h semaphore.h])
AC_CHECK_HEADER([sys/semaphore.h], [
	CPPFLAGS+=" -DHAVE_SYS_SEMAPHORE_H"
])

AC_C_INLINE
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MKTIME
AC_HEADER_STDBOOL

# run type checks
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

dnl -----------------------------------------------
dnl check for netlink library
dnl -----------------------------------------------
PKG_CHECK_MODULES([NL], [libnl-3.0], [
	AC_DEFINE(HAVE_LIBNL3, [1], ["netlink library 3.0 is available"])
	use_netlink3="yes"
	use_netlink="no"
	
	# check for additional netlink modules
	PKG_CHECK_MODULES([NL_ROUTE], [libnl-route-3.0], [
		AC_DEFINE(HAVE_LIBNL3_ROUTE, [1], ["netlink route library 3.0 is available"])
	], [
		echo "netlink route module not found"
	])
	
	# check for additional netlink modules
	PKG_CHECK_MODULES([NL_GENL], [libnl-genl-3.0], [
		AC_DEFINE(HAVE_LIBNL3_GENL, [1], ["netlink genl library 3.0 is available"])
	], [
		echo "netlink genl module not found"
	])
], [
	PKG_CHECK_MODULES([NL], [libnl-1], [
		AC_DEFINE(HAVE_LIBNL, [1], ["netlink library is available"])
		use_netlink="yes"
		use_netlink3="no"
	], [
		use_netlink="no"
		use_netlink3="no"
	])
])
AC_SUBST(NL_CFLAGS)
AC_SUBST(NL_LIBS)
AC_SUBST(NL_ROUTE_CFLAGS)
AC_SUBST(NL_ROUTE_LIBS)
AC_SUBST(NL_GENL_CFLAGS)
AC_SUBST(NL_GENL_LIBS)

dnl -----------------------------------------------
dnl check for pthread library
dnl -----------------------------------------------

THREADLIB=error

AC_CHECK_LIB(pthread, pthread_attr_init,
	THREADLIB="pthread")

if test "x$THREADLIB" = xerror; then
	AC_CHECK_LIB(pthreads, pthread_attr_init,
		THREADLIB="pthreads")
fi

if test "x$THREADLIB" = xerror; then
	AC_CHECK_LIB(thread, pthread_attr_init,
		THREADLIB="thread")
fi

PTHREAD_LIBS="-l$THREADLIB"
PTHREAD_CFLAGS=""

AC_CHECK_LIB($THREADLIB,pthread_create)
AC_CHECK_LIB($THREADLIB,pthread_mutex_init)
AC_CHECK_LIB($THREADLIB,pthread_cond_init)

AC_CHECK_LIB($THREADLIB,pthread_condattr_setclock,[
	AC_DEFINE(HAVE_PTHREAD_CONDATTR_SETCLOCK, [1], ["pthread clocking"])
])
AC_CHECK_LIB($THREADLIB,pthread_setconcurrency,[
	AC_DEFINE(HAVE_PTHREAD_SETCONCURRENCY, [1], ["pthread concurrency"])
])

AC_CHECK_LIB($THREADLIB,pthread_yield,[
	AC_DEFINE(HAVE_PTHREAD_YIELD, [1], ["pthread yield"],[
		AC_CHECK_LIB($tlib,pthread_yield_np,[
			AC_DEFINE(HAVE_PTHREAD_YIELD_NP, [1], ["pthread yield np"])
		])
	])
])

AC_CHECK_LIB($THREADLIB,pthread_delay,[
	AC_DEFINE(HAVE_PTHREAD_DELAY, [1], ["pthread delay"])
])
AC_CHECK_LIB($THREADLIB,pthread_delay_np,[
	AC_DEFINE(HAVE_PTHREAD_DELAY_NP, [1], ["pthread delay np"])
])
AC_CHECK_LIB($THREADLIB,pthread_setschedprio,[
	AC_DEFINE(HAVE_PTHREAD_SETSCHEDPRIO, [1], ["pthread scheduling"])
])

AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LIBS)

# check for openssl
AC_ARG_WITH(openssl,
	AS_HELP_STRING([--with-openssl], [Build with openssl support
		@<:@default=no@:>@]),
	[
	if test "x$with_openssl" = "xyes"; then
		PKG_CHECK_MODULES(OPENSSL, openssl, [
			use_openssl="yes"
			AC_DEFINE(HAVE_OPENSSL, [1], ["openssl library is available"])
			AC_SUBST(OPENSSL_CFLAGS)
			AC_SUBST(OPENSSL_LIBS)
		], [
			use_openssl="no"
			AC_MSG_WARN([openssl extension requested, but no openssl library is found. ssl extensions are disabled.])
		])
	else
		use_openssl="no"
	fi
	],[
	use_openssl="no"
])

# Checks for lowpan support
AC_ARG_WITH(lowpan,
	AS_HELP_STRING([--with-lowpan], [Build with lowpan support (libnl needed)
		@<:@default=no@:>@]),
	[
	if test "x$with_lowpan" = "xyes"; then
		if test "x$use_netlink" = "xyes" -o "x$use_netlink3" = "xyes"; then
			use_lowpan="yes"
			AC_DEFINE(HAVE_LOWPAN_SUPPORT, [1], ["enable lowpan support"])
		else
			use_lowpan="no"
			AC_MSG_WARN([lowpan extension requested, but no libnl library is found. lowpan support is disabled.])
		fi
	else
		use_lowpan="no"
	fi
	],[
	use_lowpan="no"
])

# check for libxml
AC_ARG_WITH(xml,
	AS_HELP_STRING([--with-xml], [Build with xml support
		@<:@default=no@:>@]),
	[
	if test "x$with_xml" = "xyes"; then
		PKG_CHECK_MODULES(LIBXML, libxml-2.0, [
			use_xml="yes"
			AC_DEFINE(HAVE_LIBXML, [1], ["xml library is available"])
			AC_SUBST(LIBXML_CFLAGS)
			AC_SUBST(LIBXML_LIBS)
		], [
			use_xml="no"
			AC_MSG_WARN([xml extension requested, but no xml library is found. xml extensions are disabled.])
		])
	else
		use_xml="no"
	fi
	],[
	use_xml="no"
])

AM_CONDITIONAL(OPENSSL, test x$use_openssl = xyes)
AM_CONDITIONAL(LOWPAN, test x$use_lowpan = xyes)
AM_CONDITIONAL(NETLINK, test x$use_netlink = xyes)
AM_CONDITIONAL(NETLINK3, test x$use_netlink3 = xyes)
AM_CONDITIONAL(LIBXML, test x$use_xml = xyes)

dnl -----------------------------------------------
dnl Generates Makefiles, configuration files and scripts
dnl -----------------------------------------------

AC_CONFIG_FILES([Makefile \
	ibrcommon.pc \
	ibrcommon/Makefile \
	ibrcommon/data/Makefile \
	ibrcommon/net/Makefile \
	ibrcommon/thread/Makefile \
	ibrcommon/ssl/Makefile \
	ibrcommon/ssl/gcm/Makefile \
	ibrcommon/xml/Makefile \
	tests/Makefile \
	tests/unittests/Makefile \
	tests/stress/Makefile])

AC_OUTPUT
